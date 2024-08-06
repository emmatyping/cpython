/*
Low level interface to Meta's zstd library for use in the `zstd` Python library.

Original implementation by Ma Lin, reworked for CPython by Ethan Smith
*/

#ifndef Py_BUILD_CORE_BUILTIN
#  define Py_BUILD_CORE_MODULE 1
#endif

#include "Python.h"

#include <zstd.h>
#include <zdict.h>

#include <stdio.h>

#include "pycore_blocks_output_buffer.h"

#if ZSTD_VERSION_NUMBER < 10400
    #error "CPython requires zstd > 1.4.0"
#endif

#pragma region "Utilities"
#define ACQUIRE_LOCK(obj) do { \
    if (!PyThread_acquire_lock((obj)->lock, 0)) { \
        Py_BEGIN_ALLOW_THREADS \
        PyThread_acquire_lock((obj)->lock, 1); \
        Py_END_ALLOW_THREADS \
    } } while (0)
#define RELEASE_LOCK(obj) PyThread_release_lock((obj)->lock)

#pragma region "blocks_buffer"
/* -----------------------------------
     Blocks output buffer wrapper code
   ----------------------------------- */

/* Initialize the buffer, and grow the buffer.
   Return 0 on success
   Return -1 on failure */
static inline int
_OutputBuffer_InitAndGrow(_BlocksOutputBuffer *buffer, ZSTD_outBuffer *ob,
                         const Py_ssize_t max_length)
{
    Py_ssize_t block_size;

    /* Ensure .list was set to NULL */
    assert(buffer->list == NULL);

    /* Get block size */
    if (0 <= max_length && max_length < BUFFER_BLOCK_SIZE[0]) {
        block_size = max_length;
    } else {
        block_size = BUFFER_BLOCK_SIZE[0];
    }
    // TODO: verify maximum size is not greater than 128KB
    Py_ssize_t res = _BlocksOutputBuffer_InitAndGrow(buffer, block_size, &ob->dst);
    if (res < 0) {
        return -1;
    }
    ob->size = (size_t) res;
    ob->pos = 0;
    return 0;
}

/* Initialize the buffer, with an initial size.
   init_size: the initial size.
   Return 0 on success
   Return -1 on failure */
static inline int
_OutputBuffer_InitWithSize(_BlocksOutputBuffer *buffer, ZSTD_outBuffer *ob,
                          const Py_ssize_t max_length,
                          const Py_ssize_t init_size)
{
    Py_ssize_t block_size;

    /* Ensure .list was set to NULL */
    assert(buffer->list == NULL);

    /* Get block size */
    if (0 <= max_length && max_length < init_size) {
        block_size = max_length;
    } else {
        block_size = init_size;
    }
    // TODO: verify maximum size is not greater than 128KB
    Py_ssize_t res = _BlocksOutputBuffer_InitWithSize(buffer, block_size, &ob->dst);
    if (res < 0) {
        return -1;
    }
    ob->size = (size_t) res;
    ob->pos = 0;
    return 0;
}

/* Grow the buffer.
   Return 0 on success
   Return -1 on failure */
static inline int
_OutputBuffer_Grow(_BlocksOutputBuffer *buffer, ZSTD_outBuffer *ob)
{
    assert(ob->pos == ob->size);
    Py_ssize_t res = _BlocksOutputBuffer_Grow(buffer, &ob->dst, 0);
    ob->size = (size_t) res;
    ob->pos = 0;
    return 0;
}

/* Finish the buffer.
   Return a bytes object on success
   Return NULL on failure */
static inline PyObject *
_OutputBuffer_Finish(_BlocksOutputBuffer *buffer, ZSTD_outBuffer *ob)
{
    return _BlocksOutputBuffer_Finish(buffer, ob->size - ob->pos);
}

/* Clean up the buffer */
static inline void
_OutputBuffer_OnError(_BlocksOutputBuffer *buffer)
{
    Py_CLEAR(buffer->list);
}
#pragma endregion
#pragma endregion

#pragma region "Module state"

typedef struct {
    PyTypeObject *zstd_compressor_type;
    //PyTypeObject *zstd_decompressor_type;
    PyTypeObject *zstd_dict_type;
    PyObject *error;
} _zstd_state;

static inline _zstd_state*
_get_zstd_state(PyObject *module)
{
    void *state = PyModule_GetState(module);
    assert(state != NULL);
    return (_zstd_state *)state;
}

// Forward declare for type lookup code below
static PyModuleDef _zstdmodule;

#pragma endregion

#pragma region "Parameters"
/* -------------------------
     Parameters from zstd
   ------------------------- */
typedef struct {
    const int parameter;
    const char parameter_name[32];
} ParameterInfo;

static const ParameterInfo cp_list[] =
{
    {ZSTD_c_compressionLevel, "compressionLevel"},
    {ZSTD_c_windowLog,        "windowLog"},
    {ZSTD_c_hashLog,          "hashLog"},
    {ZSTD_c_chainLog,         "chainLog"},
    {ZSTD_c_searchLog,        "searchLog"},
    {ZSTD_c_minMatch,         "minMatch"},
    {ZSTD_c_targetLength,     "targetLength"},
    {ZSTD_c_strategy,         "strategy"},

#if Zstd_VERSION_NUMBER >= 10506
    {ZSTD_c_targetCBlockSize, "targetCBlockSize"},
#endif

    {ZSTD_c_enableLongDistanceMatching, "enableLongDistanceMatching"},
    {ZSTD_c_ldmHashLog,       "ldmHashLog"},
    {ZSTD_c_ldmMinMatch,      "ldmMinMatch"},
    {ZSTD_c_ldmBucketSizeLog, "ldmBucketSizeLog"},
    {ZSTD_c_ldmHashRateLog,   "ldmHashRateLog"},

    {ZSTD_c_contentSizeFlag,  "contentSizeFlag"},
    {ZSTD_c_checksumFlag,     "checksumFlag"},
    {ZSTD_c_dictIDFlag,       "dictIDFlag"},

    {ZSTD_c_nbWorkers,        "nbWorkers"},
    {ZSTD_c_jobSize,          "jobSize"},
    {ZSTD_c_overlapLog,       "overlapLog"}
};

static const ParameterInfo dp_list[] =
{
    {ZSTD_d_windowLogMax, "windowLogMax"}
};

/* Format an user friendly error message. */
void
set_parameter_error(const _zstd_state* const state, int is_compress,
                    int key_v, int value_v)
{
    ParameterInfo const *list;
    int list_size;
    char const *name;
    char *type;
    ZSTD_bounds bounds;
    int i;
    char pos_msg[128];

    if (is_compress) {
        list = cp_list;
        list_size = Py_ARRAY_LENGTH(cp_list);
        type = "compression";
    } else {
        list = dp_list;
        list_size = Py_ARRAY_LENGTH(dp_list);
        type = "decompression";
    }

    /* Find parameter's name */
    name = NULL;
    for (i = 0; i < list_size; i++) {
        if (key_v == (list+i)->parameter) {
            name = (list+i)->parameter_name;
            break;
        }
    }

    /* Unknown parameter */
    if (name == NULL) {
        PyOS_snprintf(pos_msg, sizeof(pos_msg),
                      "unknown parameter (key %d)", key_v);
        name = pos_msg;
    }

    /* Get parameter bounds */
    if (is_compress) {
        bounds = ZSTD_cParam_getBounds(key_v);
    } else {
        bounds = ZSTD_dParam_getBounds(key_v);
    }
    if (ZSTD_isError(bounds.error)) {
        PyErr_Format(state->error,
                     "Zstd %s parameter \"%s\" is invalid. (zstd v%s)",
                     type, name, ZSTD_versionString());
        return;
    }

    /* Error message */
    PyErr_Format(state->error,
                 "Error when setting zstd %s parameter \"%s\", it "
                 "should %d <= value <= %d, provided value is %d. "
                 "(zstd v%s, %d-bit build)",
                 type, name,
                 bounds.lowerBound, bounds.upperBound, value_v,
                 ZSTD_versionString(), 8*(int)sizeof(Py_ssize_t));
}
#pragma endregion

#pragma region "Error handling"

typedef enum {
    ERR_DECOMPRESS,
    ERR_COMPRESS,
    ERR_SET_PLEDGED_INPUT_SIZE,

    ERR_LOAD_D_DICT,
    ERR_LOAD_C_DICT,

    ERR_GET_C_BOUNDS,
    ERR_GET_D_BOUNDS,
    ERR_SET_C_LEVEL,

    ERR_TRAIN_DICT,
    ERR_FINALIZE_DICT
} error_type;

typedef enum {
    DICT_TYPE_DIGESTED = 0,
    DICT_TYPE_UNDIGESTED = 1,
    DICT_TYPE_PREFIX = 2
} dictionary_type;

/* Format error message and set ZstdError. */
void
set_zstd_error(const _zstd_state* const state,
               const error_type type, const size_t zstd_ret)
{
    char buf[128];
    char *msg;
    assert(ZSTD_isError(zstd_ret));

    switch (type)
    {
    case ERR_DECOMPRESS:
        msg = "Unable to decompress zstd data: %s";
        break;
    case ERR_COMPRESS:
        msg = "Unable to compress zstd data: %s";
        break;
    case ERR_SET_PLEDGED_INPUT_SIZE:
        msg = "Unable to set pledged uncompressed content size: %s";
        break;

    case ERR_LOAD_D_DICT:
        msg = "Unable to load zstd dictionary or prefix for decompression: %s";
        break;
    case ERR_LOAD_C_DICT:
        msg = "Unable to load zstd dictionary or prefix for compression: %s";
        break;

    case ERR_GET_C_BOUNDS:
        msg = "Unable to get zstd compression parameter bounds: %s";
        break;
    case ERR_GET_D_BOUNDS:
        msg = "Unable to get zstd decompression parameter bounds: %s";
        break;
    case ERR_SET_C_LEVEL:
        msg = "Unable to set zstd compression level: %s";
        break;

    case ERR_TRAIN_DICT:
        msg = "Unable to train zstd dictionary: %s";
        break;
    case ERR_FINALIZE_DICT:
        msg = "Unable to finalize zstd dictionary: %s";
        break;

    default:
        Py_UNREACHABLE();
    }
    PyOS_snprintf(buf, sizeof(buf), msg, ZSTD_getErrorName(zstd_ret));
    PyErr_SetString(state->error, buf);
}
#pragma endregion

#pragma region "Objects"

typedef struct {
    PyObject_HEAD

    /* Thread lock for generating Zstd_CDict/ZSTD_DDict */
    PyThread_type_lock lock;

    /* Reusable compress/decompress dictionary, they are created once and
       can be shared by multiple threads concurrently, since its usage is
       read-only.
       c_dicts is a dict, int(compressionLevel):PyCapsule(Zstd_CDict*) */
    ZSTD_DDict *d_dict;
    PyObject *c_dicts;

    /* Content of the dictionary, bytes object. */
    PyObject *dict_content;
    /* Dictionary id */
    uint32_t dict_id;
    _zstd_state *module_state;
} ZstdDict;

typedef struct {
    PyObject_HEAD
    /* Thread lock for compressing */
    PyThread_type_lock lock;

    /* Compression context */
    ZSTD_CCtx *cctx;

    /* ZstdDict object in use */
    PyObject *dict;

    /* Last mode, initialized to ZSTD_e_end */
    int last_mode;

    /* (nbWorker >= 1) ? 1 : 0 */
    int use_multithread;

    /* Compression level */
    int compression_level;

    _zstd_state *module_state;
} ZstdCompressor;

//typedef struct {
//    PyObject_HEAD
//
//    /* Thread lock for compressing */
//    PyThread_type_lock lock;
//
//    /* Decompression context */
//    Zstd_DCtx *dctx;
//
//    /* ZstdDict object in use */
//    PyObject *dict;
//
//    /* Unconsumed input data */
//    char *input_buffer;
//    size_t input_buffer_size;
//    size_t in_begin, in_end;
//
//    /* Unused data */
//    PyObject *unused_data;
//
//    /* 0 if decompressor has (or may has) unconsumed input data, 0 or 1. */
//    char needs_input;
//
//    /* For EndlessZstdDecompressor, 0 or 1.
//       1 when both input and output streams are at a frame edge, means a
//       frame is completely decoded and fully flushed, or the decompressor
//       just be initialized. */
//    char at_frame_edge;
//
//    /* For ZstdDecompressor, 0 or 1.
//       1 means the end of the first frame has been reached. */
//    char eof;
//
//    /* Used for fast reset above three variables */
//    char _unused_char_for_align;
//
//    _zstd_state *module_state;
//} ZstdDecompressor;

#pragma endregion


/*[clinic input]
module _zstd
class _zstd.ZstdCompressor "ZstdCompressor *" ""
class _zstd.ZstdDecompressor "ZstdDecompressor *" ""
class _zstd.ZstdDict "ZstdDict *" ""
[clinic start generated code]*/
/*[clinic end generated code: output=da39a3ee5e6b4b0d input=838e9647a277ab80]*/

#include "clinic/_zstdmodule.c.h"

/* ZstdCompressor class*/
#pragma region "ZstdCompressor"

PyObject *
compress_impl(ZstdCompressor *self, Py_buffer *data,
              const ZSTD_EndDirective end_directive)
{
    ZSTD_inBuffer in;
    ZSTD_outBuffer out;
    _BlocksOutputBuffer buffer = { .list = NULL };
    size_t zstd_ret;
    PyObject *ret;

    /* Prepare input & output buffers */
    if (data != NULL) {
        in.src = data->buf;
        in.size = data->len;
        in.pos = 0;
    } else {
        in.src = &in;
        in.size = 0;
        in.pos = 0;
    }

    /* Calculate output buffer's size */
    size_t output_buffer_size = ZSTD_compressBound(in.size);
    if (output_buffer_size > (size_t) PY_SSIZE_T_MAX) {
        PyErr_NoMemory();
        goto error;
    }

    if (_OutputBuffer_InitWithSize(&buffer, &out, -1,
                                    (Py_ssize_t) output_buffer_size) < 0) {
        goto error;
    }

    /* zstd stream compress */
    while (1) {
        Py_BEGIN_ALLOW_THREADS
        zstd_ret = ZSTD_compressStream2(self->cctx, &out, &in, end_directive);
        Py_END_ALLOW_THREADS

        /* Check error */
        if (ZSTD_isError(zstd_ret)) {
            _zstd_state* const state = self->module_state;
            assert(state != NULL);
            set_zstd_error(state, ERR_COMPRESS, zstd_ret);
            goto error;
        }

        /* Finished */
        if (zstd_ret == 0) {
            break;
        }

        /* Output buffer should be exhausted, grow the buffer. */
        assert(out.pos == out.size);
        if (out.pos == out.size) {
            if (_OutputBuffer_Grow(&buffer, &out) < 0) {
                goto error;
            }
        }
    }

    /* Return a bytes object */
    ret = _OutputBuffer_Finish(&buffer, &out);
    if (ret != NULL) {
        return ret;
    }

error:
    _OutputBuffer_OnError(&buffer);
    return NULL;
}

/* In multi-thread compression + .CONTINUE mode: If input buffer exhausted,
   there may be a lot of data in internal buffer that can be outputted.
   This conditional expression output as much as possible. */
int
_mt_continue_should_break(ZSTD_inBuffer *in, ZSTD_outBuffer *out) {
    return in->size == in->pos && out->size != out->pos;
}

static PyObject *
compress_mt_continue_impl(ZstdCompressor *self, Py_buffer *data)
{
    ZSTD_inBuffer in;
    ZSTD_outBuffer out;
    _BlocksOutputBuffer buffer = { .list = NULL };
    size_t zstd_ret;
    PyObject *ret;

    /* Prepare input & output buffers */
    in.src = data->buf;
    in.size = data->len;
    in.pos = 0;

    if (_OutputBuffer_InitAndGrow(&buffer, &out, -1) < 0) {
        goto error;
    }

    /* zstd stream compress */
    while (1) {
        Py_BEGIN_ALLOW_THREADS
        do {
            zstd_ret = ZSTD_compressStream2(self->cctx, &out, &in, ZSTD_e_continue);
        } while (out.pos != out.size && in.pos != in.size && !ZSTD_isError(zstd_ret));
        Py_END_ALLOW_THREADS

        /* Check error */
        if (ZSTD_isError(zstd_ret)) {
            _zstd_state* const state = self->module_state;
            assert(state != NULL);
            set_zstd_error(state, ERR_COMPRESS, zstd_ret);
            goto error;
        }

        /* Like compress_impl(), output as much as possible. */
        if (out.pos == out.size) {
            if (_OutputBuffer_Grow(&buffer, &out) < 0) {
                goto error;
            }
        } else if (in.pos == in.size) {
            /* Finished */
            assert(_mt_continue_should_break(&in, &out));
            break;
        }
    }

    /* Return a bytes object */
    ret = _OutputBuffer_Finish(&buffer, &out);
    if (ret != NULL) {
        return ret;
    }

error:
    _OutputBuffer_OnError(&buffer);
    return NULL;
}


/*[clinic input]
_zstd.ZstdCompressor.compress

    data: Py_buffer
    mode: int
        Can be these 3 values ZstdCompressor.CONTINUE,
        ZstdCompressor.FLUSH_BLOCK, ZstdCompressor.FLUSH_FRAME
    /

Provide data to the compressor object.

Return a chunk of compressed data if possible, or b'' otherwise.
[clinic start generated code]*/

static PyObject *
_zstd_ZstdCompressor_compress_impl(ZstdCompressor *self, Py_buffer *data,
                                   int mode)
/*[clinic end generated code: output=ed7982d1cf7b4f98 input=90409b6b4037aaf2]*/
{
    PyObject *ret;
    /* Check mode value */
    if (mode != ZSTD_e_continue &&
        mode != ZSTD_e_flush &&
        mode != ZSTD_e_end)
    {
        PyErr_SetString(PyExc_ValueError,
                        "mode argument wrong value, it should be one of "
                        "ZstdCompressor.CONTINUE, ZstdCompressor.FLUSH_BLOCK, "
                        "ZstdCompressor.FLUSH_FRAME.");
        PyBuffer_Release(data);
        return NULL;
    }

    /* Thread-safe code */
    ACQUIRE_LOCK(self);

    /* Compress */
    if (self->use_multithread && mode == ZSTD_e_continue) {
        ret = compress_mt_continue_impl(self, data);
    } else {
        ret = compress_impl(self, data, mode);
    }

    if (ret) {
        self->last_mode = mode;
    } else {
        self->last_mode = ZSTD_e_end;

        /* Resetting cctx's session never fail */
        ZSTD_CCtx_reset(self->cctx, ZSTD_reset_session_only);
    }
    RELEASE_LOCK(self);

    PyBuffer_Release(data);
    return ret;
}

/*[clinic input]
_zstd.ZstdCompressor.flush

    mode: int
        Can be these 2 values ZstdCompressor.FLUSH_FRAME,
        ZstdCompressor.FLUSH_BLOCK
    /

Flush any remaining data in internal buffer.

Since zstd data consists of one or more independent frames, the compressor
object can still be used after this method is called.
[clinic start generated code]*/

static PyObject *
_zstd_ZstdCompressor_flush_impl(ZstdCompressor *self, int mode)
/*[clinic end generated code: output=b7cf2c8d64dcf2e3 input=d7568fbb5e5165f5]*/
{
    PyObject *ret;

    /* Check mode value */
    if (mode != ZSTD_e_end && mode != ZSTD_e_flush) {
        PyErr_SetString(PyExc_ValueError,
                        "mode argument wrong value, it should be "
                        "ZstdCompressor.FLUSH_FRAME or "
                        "ZstdCompressor.FLUSH_BLOCK.");
        return NULL;
    }

    /* Thread-safe code */
    ACQUIRE_LOCK(self);
    ret = compress_impl(self, NULL, mode);

    if (ret) {
        self->last_mode = mode;
    } else {
        self->last_mode = ZSTD_e_end;

        /* Resetting cctx's session never fail */
        ZSTD_CCtx_reset(self->cctx, ZSTD_reset_session_only);
    }
    RELEASE_LOCK(self);

    return ret;
}

/*[-clinic input]
@classmethod
_zstd.ZstdCompressor.__new__

[-clinic start generated code]*/
static PyObject *
_zstd_ZstdCompressor_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
    ZstdCompressor *self;
    self = (ZstdCompressor*)type->tp_alloc(type, 0);
    if (self == NULL) {
        goto error;
    }

    /* Keep this first. Set module state to self. */
    (self)->module_state = (_zstd_state*)PyType_GetModuleState(type);
    if ((self)->module_state == NULL) {
        goto error;
    }

    assert(self->dict == NULL);
    assert(self->use_multithread == 0);
    assert(self->compression_level == 0);

    /* Compression context */
    self->cctx = ZSTD_createCCtx();
    if (self->cctx == NULL) {
        _zstd_state *state = PyType_GetModuleState(type);
        PyErr_SetString(state->error,
                        "Unable to create ZSTD_CCtx instance.");
        goto error;
    }

    /* Last mode */
    self->last_mode = ZSTD_e_end;

    /* Thread lock */
    self->lock = PyThread_allocate_lock();
    if (self->lock == NULL) {
        PyErr_NoMemory();
        goto error;
    }
    return (PyObject*)self;

error:
    Py_XDECREF(self);
    return NULL;
}

static void
capsule_free_cdict(PyObject *capsule)
{
    ZSTD_CDict *cdict = PyCapsule_GetPointer(capsule, NULL);
    ZSTD_freeCDict(cdict);
}

static inline ZSTD_CDict *
_get_CDict(ZstdDict *self, int compressionLevel)
{
    PyObject *level = NULL;
    PyObject *capsule;
    ZSTD_CDict *cdict;

    ACQUIRE_LOCK(self);

    /* int level object */
    level = PyLong_FromLong(compressionLevel);
    if (level == NULL) {
        goto error;
    }

    /* Get PyCapsule object from self->c_dicts */
    capsule = PyDict_GetItemWithError(self->c_dicts, level);
    if (capsule == NULL) {
        if (PyErr_Occurred()) {
            goto error;
        }

        /* Create ZSTD_CDict instance */
        Py_BEGIN_ALLOW_THREADS
        cdict = ZSTD_createCDict(PyBytes_AS_STRING(self->dict_content),
                                 Py_SIZE(self->dict_content), compressionLevel);
        Py_END_ALLOW_THREADS

        if (cdict == NULL) {
            _zstd_state *state = self->module_state;
            PyErr_SetString(state->error,
                            "Failed to create ZSTD_CDict instance from zstd "
                            "dictionary content. Maybe the content is corrupted.");
            goto error;
        }

        /* Put ZSTD_CDict instance into PyCapsule object */
        capsule = PyCapsule_New(cdict, NULL, capsule_free_cdict);
        if (capsule == NULL) {
            ZSTD_freeCDict(cdict);
            goto error;
        }

        /* Add PyCapsule object to self->c_dicts */
        if (PyDict_SetItem(self->c_dicts, level, capsule) < 0) {
            Py_DECREF(capsule);
            goto error;
        }
        Py_DECREF(capsule);
    } else {
        /* ZSTD_CDict instance already exists */
        cdict = PyCapsule_GetPointer(capsule, NULL);
    }
    goto success;

error:
    cdict = NULL;
success:
    Py_XDECREF(level);
    RELEASE_LOCK(self);
    return cdict;
}

static inline ZSTD_DDict *
_get_DDict(ZstdDict *self)
{
    ZSTD_DDict *ret;

    /* Already created */
    if (self->d_dict != NULL) {
        return self->d_dict;
    }

    ACQUIRE_LOCK(self);
    if (self->d_dict == NULL) {
        /* Create ZSTD_DDict instance from dictionary content */
        Py_BEGIN_ALLOW_THREADS
        self->d_dict = ZSTD_createDDict(PyBytes_AS_STRING(self->dict_content),
                                        Py_SIZE(self->dict_content));
        Py_END_ALLOW_THREADS

        if (self->d_dict == NULL) {
            _zstd_state *state = self->module_state;
            PyErr_SetString(state->error,
                            "Failed to create ZSTD_DDict instance from zstd "
                            "dictionary content. Maybe the content is corrupted.");
        }
    }

    /* Don't lose any exception */
    ret = self->d_dict;
    RELEASE_LOCK(self);

    return ret;
}

static int
_load_c_dict(ZstdCompressor *self, PyObject *dict)
{
    size_t zstd_ret;
    _zstd_state *state = self->module_state;
    ZstdDict *zd;
    int type, ret;

    /* Check ZstdDict */
    ret = PyObject_IsInstance(dict, (PyObject*)state->zstd_dict_type);
    if (ret < 0) {
        return -1;
    } else if (ret > 0) {
        /* When compressing, use undigested dictionary by default. */
        zd = (ZstdDict*)dict;
        type = DICT_TYPE_UNDIGESTED;
        goto load;
    }

    /* Check (ZstdDict, type) */
    if (PyTuple_CheckExact(dict) && PyTuple_GET_SIZE(dict) == 2) {
        /* Check ZstdDict */
        ret = PyObject_IsInstance(PyTuple_GET_ITEM(dict, 0),
                                  (PyObject*)state->zstd_dict_type);
        if (ret < 0) {
            return -1;
        } else if (ret > 0) {
            /* type == -1 may indicate an error. */
            type = PyLong_AsInt(PyTuple_GET_ITEM(dict, 1));
            if (type == DICT_TYPE_DIGESTED ||
                type == DICT_TYPE_UNDIGESTED ||
                type == DICT_TYPE_PREFIX)
            {
                assert(type >= 0);
                zd = (ZstdDict*)PyTuple_GET_ITEM(dict, 0);
                goto load;
            }
        }
    }

    /* Wrong type */
    PyErr_SetString(PyExc_TypeError,
                    "zstd_dict argument should be ZstdDict object.");
    return -1;

load:
    if (type == DICT_TYPE_DIGESTED) {
        /* Get ZSTD_CDict */
        ZSTD_CDict *c_dict = _get_CDict(zd, self->compression_level);
        if (c_dict == NULL) {
            return -1;
        }
        /* Reference a prepared dictionary.
           It overrides some compression context's parameters. */
        zstd_ret = ZSTD_CCtx_refCDict(self->cctx, c_dict);
    } else if (type == DICT_TYPE_UNDIGESTED) {
        /* Load a dictionary.
           It doesn't override compression context's parameters. */
        zstd_ret = ZSTD_CCtx_loadDictionary(
                            self->cctx,
                            PyBytes_AS_STRING(zd->dict_content),
                            Py_SIZE(zd->dict_content));
    } else if (type == DICT_TYPE_PREFIX) {
        /* Load a prefix */
        zstd_ret = ZSTD_CCtx_refPrefix(
                            self->cctx,
                            PyBytes_AS_STRING(zd->dict_content),
                            Py_SIZE(zd->dict_content));
    } else {
        /* Impossible code path */
        PyErr_SetString(PyExc_SystemError,
                        "load_c_dict() impossible code path");
        return -1;
    }

    /* Check error */
    if (ZSTD_isError(zstd_ret)) {
        _zstd_state *state = self->module_state;
        set_zstd_error(state, ERR_LOAD_C_DICT, zstd_ret);
        return -1;
    }
    return 0;
}

/*[clinic input]
_zstd.ZstdCompressor.__init__
    compression_level: int = 0
        Compression level to use.
    options: object = NULL
        A mapping of advanced compression parameters.
    zstd_dict: object = NULL
        A ZstdDict object, pre-trained zstd dictionary.

Create a streaming compressor object for compressing data incrementally.

For one-shot compression, use the compress() function instead.
[clinic start generated code]*/

static int
_zstd_ZstdCompressor___init___impl(ZstdCompressor *self,
                                   int compression_level, PyObject *options,
                                   PyObject *zstd_dict)
/*[clinic end generated code: output=4a6bec6c6bc295d5 input=27bd367d2c9cfd27]*/
{
    size_t zstd_ret;
    self->compression_level = compression_level;

    /* Save for generating Zstd_CDICT */
    self->compression_level = compression_level;

    /* Set compressionLevel to compression context */
    zstd_ret = ZSTD_CCtx_setParameter(self->cctx,
                                        ZSTD_c_compressionLevel,
                                        compression_level);
    if (ZSTD_isError(zstd_ret)) {
        set_zstd_error(self->module_state, ERR_SET_C_LEVEL, zstd_ret);
        return -1;
    }

    // Handle options if not NULL
    if (options != NULL) {
        PyObject *key, *value;
        Py_ssize_t pos = 0;

        while (PyDict_Next(options, &pos, &key, &value)) {
            /* Check key type */
            /* if (Py_TYPE(key) == self->module_state->dparameter_type) {
                PyErr_SetString(PyExc_TypeError,
                                "Key of compression option dict should "
                                "NOT be DParameter.");
                return -1;
            } */

            /* Both key & value should be 32-bit signed int */
            const int key_v = PyLong_AsInt(key);
            if (key_v == -1 && PyErr_Occurred()) {
                PyErr_SetString(PyExc_ValueError,
                                "Key of option dict should be 32-bit signed int value.");
                return -1;
            }

            const int value_v = PyLong_AsInt(value);
            if (value_v == -1 && PyErr_Occurred()) {
                PyErr_SetString(PyExc_ValueError,
                                "Value of option dict should be 32-bit signed int value.");
                return -1;
            }

            if (key_v == ZSTD_c_compressionLevel) {
                /* Save for generating Zstd_CDICT */
                self->compression_level = value_v;
            } else if (key_v == ZSTD_c_nbWorkers) {
                /* From zstd library doc:
                   1. When nbWorkers >= 1, triggers asynchronous mode when
                      used with ZSTD_compressStream2().
                   2, Default value is `0`, aka "single-threaded mode" : no
                      worker is spawned, compression is performed inside
                      caller's thread, all invocations are blocking. */
                if (value_v != 0) {
                    self->use_multithread = 1;
                }
            }

            /* Set parameter to compression context */
            zstd_ret = ZSTD_CCtx_setParameter(self->cctx, key_v, value_v);
            if (ZSTD_isError(zstd_ret)) {
                set_parameter_error(self->module_state, 1, key_v, value_v);
                return -1;
            }
        }
    }
    
    /* Set compressLevel/option to compression context */
    //if (level_or_option != Py_None) {
    //    if (set_c_parameters(self, level_or_option) < 0) {
    //        return -1;
    //    }
    //}

    /* Load dictionary to compression context */
    if (zstd_dict != NULL) {
        if (_load_c_dict(self, zstd_dict) < 0) {
            return -1;
        }

        /* Py_INCREF the dict */
        Py_INCREF(zstd_dict);
        self->dict = zstd_dict;
    }

    return 0;
}

static void
Compressor_dealloc(ZstdCompressor *self)
{
    /* Free compression context */
    ZSTD_freeCCtx(self->cctx);

    /* Py_XDECREF the dict after free the compression context */
    Py_XDECREF(self->dict);

    /* Thread lock */
    if (self->lock) {
        PyThread_free_lock(self->lock);
    }

    PyTypeObject *tp = Py_TYPE(self);
    tp->tp_free((PyObject*)self);
    Py_DECREF(tp);
}

static PyMethodDef Compressor_methods[] = {
    _ZSTD_ZSTDCOMPRESSOR_COMPRESS_METHODDEF
    _ZSTD_ZSTDCOMPRESSOR_FLUSH_METHODDEF
    {NULL}
};

static int
Compressor_traverse(ZstdCompressor *self, visitproc visit, void *arg)
{
    Py_VISIT(Py_TYPE(self));
    return 0;
}

PyDoc_STRVAR(Compressor_doc,
"A streaming compressor. Thread-safe at method level.\n\n"
"ZstdCompressor.__init__(self, level=None, option=None, zstd_dict=None)\n"
"----\n"
"Initialize a ZstdCompressor object.\n\n"
"Parameters\n"
"level:           the compression level.\n"
"options:          advanced compression parameters.\n"
"zstd_dict:       A ZstdDict object, pre-trained zstd dictionary.");

static PyType_Slot zstd_compressor_type_slots[] = {
    {Py_tp_dealloc, Compressor_dealloc},
    {Py_tp_methods, Compressor_methods},
    {Py_tp_init, _zstd_ZstdCompressor___init__},
    {Py_tp_new, _zstd_ZstdCompressor_new},
    {Py_tp_doc, (char *)Compressor_doc},
    {Py_tp_traverse, Compressor_traverse},
    {0, 0}
};

static PyType_Spec zstd_compressor_type_spec = {
    .name = "_zstd.ZstdCompressor",
    .basicsize = sizeof(ZstdCompressor),
    // Calling PyType_GetModuleState() on a subclass is not safe.
    // zstd_compressor_type_spec does not have Py_TPFLAGS_BASETYPE flag
    // which prevents to create a subclass.
    // So calling PyType_GetModuleState() in this file is always safe.
    .flags = Py_TPFLAGS_DEFAULT,
    .slots = zstd_compressor_type_slots,
};

#pragma endregion

#pragma region "ZstdDict"

/* -----------------
     ZstdDict code
   ----------------- */

/*[-clinic input]
@classmethod
_zstd.ZstdDict.__new__


Return a new ZstdDict object.
[-clinic start generated code]*/

static PyObject *
_zstd_ZstdDict_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
    ZstdDict *self;
    self = (ZstdDict*)type->tp_alloc(type, 0);
    if (self == NULL) {
        goto error;
    }

    /* Keep this first. Set module state to self. */
    (self)->module_state = (_zstd_state*)PyType_GetModuleState(type);
    if ((self)->module_state == NULL) {
        goto error;
    }

    assert(self->dict_content == NULL);
    assert(self->dict_id == 0);
    assert(self->d_dict == NULL);
    //assert(self->inited == 0);

    /* Zstd_CDict dict */
    self->c_dicts = PyDict_New();
    if (self->c_dicts == NULL) {
        goto error;
    }

    /* Thread lock */
    self->lock = PyThread_allocate_lock();
    if (self->lock == NULL) {
        PyErr_NoMemory();
        goto error;
    }
    return (PyObject*)self;

error:
    Py_XDECREF(self);
    return NULL;
}

static void
ZstdDict_dealloc(ZstdDict *self)
{
    /* Free Zstd_CDict instances */
    Py_XDECREF(self->c_dicts);

    /* Free ZSTD_DDict instance */
    ZSTD_freeDDict(self->d_dict);

    /* Release dict_content after Free Zstd_CDict/ZSTD_DDict instances */
    Py_XDECREF(self->dict_content);

    /* Free thread lock */
    if (self->lock) {
        PyThread_free_lock(self->lock);
    }

    PyTypeObject *tp = Py_TYPE(self);
    tp->tp_free((PyObject*)self);
    Py_DECREF(tp);
}

/*[clinic input]
_zstd.ZstdDict.__init__
    dict_content: object
        A bytes-like object, dictionary's content.
    is_raw: bool = False
        This parameter is for advanced users. True means dict_content
        argument is a "raw content" dictionary, free of any format
        restriction. False means dict_content argument is an ordinary
        zstd dictionary, was created by zstd functions, follow a
        specified format.

Zstd dictionary, used for compression/decompression.
[clinic start generated code]*/

static int
_zstd_ZstdDict___init___impl(ZstdDict *self, PyObject *dict_content,
                             int is_raw)
/*[clinic end generated code: output=c5f5a0d8377d037c input=a2f29e8b52cbee62]*/
{
    /* Check dict_content's type */
    self->dict_content = PyBytes_FromObject(dict_content);
    if (self->dict_content == NULL) {
        PyErr_SetString(PyExc_TypeError,
                        "dict_content argument should be bytes-like object.");
        return -1;
    }

    /* Both ordinary dictionary and "raw content" dictionary should
       at least 8 bytes */
    if (Py_SIZE(self->dict_content) < 8) {
        PyErr_SetString(PyExc_ValueError,
                        "Zstd dictionary content should at least 8 bytes.");
        return -1;
    }

    /* Get dict_id, 0 means "raw content" dictionary. */
    self->dict_id = ZSTD_getDictID_fromDict(PyBytes_AS_STRING(self->dict_content),
                                            Py_SIZE(self->dict_content));

    /* Check validity for ordinary dictionary */
    if (!is_raw && self->dict_id == 0) {
        char *msg = "The dict_content argument is not a valid zstd "
                    "dictionary. The first 4 bytes of a valid zstd dictionary "
                    "should be a magic number: b'\\x37\\xA4\\x30\\xEC'.\n"
                    "If you are an advanced user, and can be sure that "
                    "dict_content argument is a \"raw content\" zstd "
                    "dictionary, set is_raw parameter to True.";
        PyErr_SetString(PyExc_ValueError, msg);
        return -1;
    }

    return 0;
}

PyDoc_STRVAR(ZstdDict_dict_doc,
"Zstd dictionary, used for compression/decompression.\n\n"
"ZstdDict.__init__(self, dict_content, is_raw=False)\n"
"----\n"
"Initialize a ZstdDict object.\n\n"
"Parameters\n"
"dict_content: A bytes-like object, dictionary's content.\n"
"is_raw:       This parameter is for advanced user. True means dict_content\n"
"              argument is a \"raw content\" dictionary, free of any format\n"
"              restriction. False means dict_content argument is an ordinary\n"
"              zstd dictionary, was created by zstd functions, follow a\n"
"              specified format.");

PyDoc_STRVAR(ZstdDict_dictid_doc,
"ID of zstd dictionary, a 32-bit unsigned int value.\n\n"
"Non-zero means ordinary dictionary, was created by zstd functions, follow\n"
"a specified format.\n\n"
"0 means a \"raw content\" dictionary, free of any format restriction, used\n"
"for advanced user.");

PyDoc_STRVAR(ZstdDict_dictcontent_doc,
"The content of zstd dictionary, a bytes object, it's the same as dict_content\n"
"argument in ZstdDict.__init__() method. It can be used with other programs.");

static PyObject *
ZstdDict_str(ZstdDict *dict)
{
    char buf[64];
    PyOS_snprintf(buf, sizeof(buf),
                  "<ZstdDict dict_id=%u dict_size=%zd>",
                  dict->dict_id, Py_SIZE(dict->dict_content));

    return PyUnicode_FromString(buf);
}

static PyMemberDef ZstdDict_members[] = {
    {"dict_id", Py_T_UINT, offsetof(ZstdDict, dict_id), Py_READONLY, ZstdDict_dictid_doc},
    {"dict_content", Py_T_OBJECT_EX, offsetof(ZstdDict, dict_content), Py_READONLY, ZstdDict_dictcontent_doc},
    {0}
};

PyDoc_STRVAR(ZstdDict_as_digested_dict_doc,
"Load as a digested dictionary to compressor, by passing this attribute as\n"
"zstd_dict argument: compress(dat, zstd_dict=zd.as_digested_dict)\n"
"1, Some advanced compression parameters of compressor may be overridden\n"
"   by parameters of digested dictionary.\n"
"2, ZstdDict has a digested dictionaries cache for each compression level.\n"
"   It's faster when loading again a digested dictionary with the same\n"
"   compression level.\n"
"3, No need to use this for decompression.");

static PyObject *
ZstdDict_as_digested_dict_get(ZstdDict *self, void *Py_UNUSED(ignored))
{
    return Py_BuildValue("Oi", self, DICT_TYPE_DIGESTED);
}

PyDoc_STRVAR(ZstdDict_as_undigested_dict_doc,
"Load as an undigested dictionary to compressor, by passing this attribute as\n"
"zstd_dict argument: compress(dat, zstd_dict=zd.as_undigested_dict)\n"
"1, The advanced compression parameters of compressor will not be overridden.\n"
"2, Loading an undigested dictionary is costly. If load an undigested dictionary\n"
"   multiple times, consider reusing a compressor object.\n"
"3, No need to use this for decompression.");

static PyObject *
ZstdDict_as_undigested_dict_get(ZstdDict *self, void *Py_UNUSED(ignored))
{
    return Py_BuildValue("Oi", self, DICT_TYPE_UNDIGESTED);
}

PyDoc_STRVAR(ZstdDict_as_prefix_doc,
"Load as a prefix to compressor/decompressor, by passing this attribute as\n"
"zstd_dict argument: compress(dat, zstd_dict=zd.as_prefix)\n"
"1, Prefix is compatible with long distance matching, while dictionary is not.\n"
"2, It only works for the first frame, then the compressor/decompressor will\n"
"   return to no prefix state.\n"
"3, When decompressing, must use the same prefix as when compressing.");

static PyObject *
ZstdDict_as_prefix_get(ZstdDict *self, void *Py_UNUSED(ignored))
{
    return Py_BuildValue("Oi", self, DICT_TYPE_PREFIX);
}

static PyGetSetDef ZstdDict_getset[] = {
    {"as_digested_dict", (getter)ZstdDict_as_digested_dict_get,
     NULL, ZstdDict_as_digested_dict_doc},

    {"as_undigested_dict", (getter)ZstdDict_as_undigested_dict_get,
     NULL, ZstdDict_as_undigested_dict_doc},

    {"as_prefix", (getter)ZstdDict_as_prefix_get,
     NULL, ZstdDict_as_prefix_doc},

    {0}
};

static Py_ssize_t
ZstdDict_length(ZstdDict *self)
{
    assert(PyBytes_Check(self->dict_content));
    return Py_SIZE(self->dict_content);
}

static PyType_Slot zstddict_slots[] = {
    {Py_tp_members, ZstdDict_members},
    {Py_tp_getset, ZstdDict_getset},
    {Py_tp_new, _zstd_ZstdDict_new},
    {Py_tp_dealloc, ZstdDict_dealloc},
    {Py_tp_init, _zstd_ZstdDict___init__},
    {Py_tp_str, ZstdDict_str},
    {Py_tp_doc, (char*)ZstdDict_dict_doc},
    {Py_sq_length, ZstdDict_length},
    {0}
};

static PyType_Spec zstddict_type_spec = {
    .name = "_zstd.ZstdDict",
    .basicsize = sizeof(ZstdDict),
    .flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .slots = zstddict_slots,
};
#pragma endregion

#pragma region "module"

#pragma region "module-level functions"
/* Module-level functions. */

#pragma region "dict training"

/* -------------------------
     Train dictionary code
   ------------------------- */

/*[clinic input]
_zstd._train_dict
    
        samples_bytes: PyBytesObject
            Concatenation of samples.
        samples_size_list: object(subclass_of='&PyList_Type')
            List of samples' sizes.
        dict_size: Py_ssize_t
            The size of the dictionary.
        /

Internal function, train a zstd dictionary.
[clinic start generated code]*/

static PyObject *
_zstd__train_dict_impl(PyObject *module, PyBytesObject *samples_bytes,
                       PyObject *samples_size_list, Py_ssize_t dict_size)
/*[clinic end generated code: output=ee53c34c8f77886b input=9eb0ccc212bc2b1f]*/
{
    Py_ssize_t chunks_number;
    size_t *chunk_sizes = NULL;
    PyObject *dst_dict_bytes = NULL;
    size_t zstd_ret;
    Py_ssize_t sizes_sum;
    Py_ssize_t i;

    /* Check arguments */
    if (dict_size <= 0) {
        PyErr_SetString(PyExc_ValueError, "dict_size argument should be positive number.");
        return NULL;
    }

    if (!PyList_Check(samples_size_list)) {
        PyErr_SetString(PyExc_TypeError,
                        "samples_size_list argument should be a list.");
        return NULL;
    }

    chunks_number = Py_SIZE(samples_size_list);
    if ((size_t) chunks_number > UINT32_MAX) {
        PyErr_SetString(PyExc_ValueError,
                        "The number of samples should <= UINT32_MAX.");
        return NULL;
    }

    /* Prepare chunk_sizes */
    chunk_sizes = PyMem_Malloc(chunks_number * sizeof(size_t));
    if (chunk_sizes == NULL) {
        PyErr_NoMemory();
        goto error;
    }

    sizes_sum = 0;
    for (i = 0; i < chunks_number; i++) {
        PyObject *size = PyList_GET_ITEM(samples_size_list, i);
        chunk_sizes[i] = PyLong_AsSize_t(size);
        if (chunk_sizes[i] == (size_t)-1 && PyErr_Occurred()) {
            PyErr_SetString(PyExc_ValueError,
                            "Items in samples_size_list should be an int "
                            "object, with a size_t value.");
            goto error;
        }
        sizes_sum += chunk_sizes[i];
    }

    if (sizes_sum != Py_SIZE(samples_bytes)) {
        PyErr_SetString(PyExc_ValueError,
                        "The samples size list doesn't match the concatenation's size.");
        goto error;
    }

    /* Allocate dict buffer */
    dst_dict_bytes = PyBytes_FromStringAndSize(NULL, dict_size);
    if (dst_dict_bytes == NULL) {
        goto error;
    }

    /* Train the dictionary */
    Py_BEGIN_ALLOW_THREADS
    zstd_ret = ZDICT_trainFromBuffer(PyBytes_AS_STRING(dst_dict_bytes), dict_size,
                                     PyBytes_AS_STRING(samples_bytes),
                                     chunk_sizes, (uint32_t)chunks_number);
    Py_END_ALLOW_THREADS

    /* Check zstd dict error */
    if (ZDICT_isError(zstd_ret)) {
        _zstd_state* const state = _get_zstd_state(module);
        set_zstd_error(state, ERR_TRAIN_DICT, zstd_ret);
        goto error;
    }

    /* Resize dict_buffer */
    if (_PyBytes_Resize(&dst_dict_bytes, zstd_ret) < 0) {
        goto error;
    }

    goto success;

error:
    Py_CLEAR(dst_dict_bytes);

success:
    PyMem_Free(chunk_sizes);
    return dst_dict_bytes;
}

/*[clinic input]
_zstd._finalize_dict
    
        custom_dict_bytes: PyBytesObject
            Custom dictionary content.
        samples_bytes: PyBytesObject
            Concatenation of samples.
        samples_size_list: object(subclass_of='&PyList_Type')
            List of samples' sizes.
        dict_size: Py_ssize_t
            The size of the dictionary.
        compression_level: int
            Optimize for a specific zstd compression level, 0 means default.
        /

Internal function, finalize a zstd dictionary.
[clinic start generated code]*/

static PyObject *
_zstd__finalize_dict_impl(PyObject *module, PyBytesObject *custom_dict_bytes,
                          PyBytesObject *samples_bytes,
                          PyObject *samples_size_list, Py_ssize_t dict_size,
                          int compression_level)
/*[clinic end generated code: output=9c2a7d8c845cee93 input=0904bda11cdb062f]*/
{
#if Zstd_VERSION_NUMBER < 10405
    PyErr_Format(PyExc_NotImplementedError,
                 "_finalize_dict function only available when the underlying "
                 "zstd library's version is greater than or equal to v1.4.5. "
                 "At pyzstd module's compile-time, zstd version < v1.4.5. At "
                 "pyzstd module's run-time, zstd version is v%s.",
                 ZSTD_versionString());
    return NULL;
#else
    Py_ssize_t chunks_number;
    size_t *chunk_sizes = NULL;
    PyObject *dst_dict_bytes = NULL;
    size_t zstd_ret;
    ZDICT_params_t params;
    Py_ssize_t sizes_sum;
    Py_ssize_t i;

    /* Check arguments */
    if (dict_size <= 0) {
        PyErr_SetString(PyExc_ValueError, "dict_size argument should be positive number.");
        return NULL;
    }

    if (!PyList_Check(samples_size_list)) {
        PyErr_SetString(PyExc_TypeError,
                        "samples_size_list argument should be a list.");
        return NULL;
    }

    chunks_number = Py_SIZE(samples_size_list);
    if ((size_t) chunks_number > UINT32_MAX) {
        PyErr_SetString(PyExc_ValueError,
                        "The number of samples should <= UINT32_MAX.");
        return NULL;
    }

    /* Prepare chunk_sizes */
    chunk_sizes = PyMem_Malloc(chunks_number * sizeof(size_t));
    if (chunk_sizes == NULL) {
        PyErr_NoMemory();
        goto error;
    }

    sizes_sum = 0;
    for (i = 0; i < chunks_number; i++) {
        PyObject *size = PyList_GET_ITEM(samples_size_list, i);
        chunk_sizes[i] = PyLong_AsSize_t(size);
        if (chunk_sizes[i] == (size_t)-1 && PyErr_Occurred()) {
            PyErr_SetString(PyExc_ValueError,
                            "Items in samples_size_list should be an int "
                            "object, with a size_t value.");
            goto error;
        }
        sizes_sum += chunk_sizes[i];
    }

    if (sizes_sum != Py_SIZE(samples_bytes)) {
        PyErr_SetString(PyExc_ValueError,
                        "The samples size list doesn't match the concatenation's size.");
        goto error;
    }

    /* Allocate dict buffer */
    dst_dict_bytes = PyBytes_FromStringAndSize(NULL, dict_size);
    if (dst_dict_bytes == NULL) {
        goto error;
    }

    /* Parameters */

    /* Optimize for a specific zstd compression level, 0 means default. */
    params.compressionLevel = compression_level;
    /* Write log to stderr, 0 = none. */
    params.notificationLevel = 0;
    /* Force dictID value, 0 means auto mode (32-bits random value). */
    params.dictID = 0;

    /* Finalize the dictionary */
    Py_BEGIN_ALLOW_THREADS
    zstd_ret = ZDICT_finalizeDictionary(
                        PyBytes_AS_STRING(dst_dict_bytes), dict_size,
                        PyBytes_AS_STRING(custom_dict_bytes), Py_SIZE(custom_dict_bytes),
                        PyBytes_AS_STRING(samples_bytes), chunk_sizes,
                        (uint32_t)chunks_number, params);
    Py_END_ALLOW_THREADS

    /* Check zstd dict error */
    if (ZDICT_isError(zstd_ret)) {
        _zstd_state* const state = _get_zstd_state(module);
        set_zstd_error(state, ERR_FINALIZE_DICT, zstd_ret);
        goto error;
    }

    /* Resize dict_buffer */
    if (_PyBytes_Resize(&dst_dict_bytes, zstd_ret) < 0) {
        goto error;
    }

    goto success;

error:
    Py_CLEAR(dst_dict_bytes);

success:
    PyMem_Free(chunk_sizes);
    return dst_dict_bytes;
#endif
}
#pragma endregion

/*[clinic input]
_zstd._get_cparam_bounds
    parameter: int
    /

Get CParameter bounds.
[clinic start generated code]*/

static PyObject *
_zstd__get_cparam_bounds_impl(PyObject *module, int parameter)
/*[clinic end generated code: output=a2a3222994523a4b input=423cb28c358de356]*/
{
    ZSTD_bounds bound = ZSTD_cParam_getBounds(parameter);
    if (ZSTD_isError(bound.error)) {
        _zstd_state *state = _get_zstd_state(module);
        set_zstd_error(state, ERR_GET_C_BOUNDS, bound.error);
        return NULL;
    }

    return Py_BuildValue("ii", bound.lowerBound, bound.upperBound);
}

/*[clinic input]
_zstd._get_dparam_bounds
    parameter: int
    /

Get DParameter bounds.
[clinic start generated code]*/

static PyObject *
_zstd__get_dparam_bounds_impl(PyObject *module, int parameter)
/*[clinic end generated code: output=fda519067c9998ec input=ce23995d895d5460]*/
{

    ZSTD_bounds bound = ZSTD_dParam_getBounds(parameter);
    if (ZSTD_isError(bound.error)) {
        _zstd_state *state = _get_zstd_state(module);
        set_zstd_error(state, ERR_GET_D_BOUNDS, bound.error);
        return NULL;
    }


    return Py_BuildValue("ii", bound.lowerBound, bound.upperBound);
}

#pragma endregion

#define ADD_INT_PREFIX_MACRO(module, macro)                           \
    do {                                                              \
        if (PyModule_AddIntConstant(module, "_" #macro, macro) < 0) { \
            return -1;                                                \
        }                                                             \
    } while(0)

static int
add_parameters(PyObject *module)
{
    /* If add new parameters, please also add to cp_list/dp_list above. */

    /* Compression parameters */
    ADD_INT_PREFIX_MACRO(module, ZSTD_c_compressionLevel);
    ADD_INT_PREFIX_MACRO(module, ZSTD_c_windowLog);
    ADD_INT_PREFIX_MACRO(module, ZSTD_c_hashLog);
    ADD_INT_PREFIX_MACRO(module, ZSTD_c_chainLog);
    ADD_INT_PREFIX_MACRO(module, ZSTD_c_searchLog);
    ADD_INT_PREFIX_MACRO(module, ZSTD_c_minMatch);
    ADD_INT_PREFIX_MACRO(module, ZSTD_c_targetLength);
    ADD_INT_PREFIX_MACRO(module, ZSTD_c_strategy);
#if ZSTD_VERSION_NUMBER >= 10506
    ADD_INT_PREFIX_MACRO(module, ZSTD_c_targetCBlockSize);
#endif

    ADD_INT_PREFIX_MACRO(module, ZSTD_c_enableLongDistanceMatching);
    ADD_INT_PREFIX_MACRO(module, ZSTD_c_ldmHashLog);
    ADD_INT_PREFIX_MACRO(module, ZSTD_c_ldmMinMatch);
    ADD_INT_PREFIX_MACRO(module, ZSTD_c_ldmBucketSizeLog);
    ADD_INT_PREFIX_MACRO(module, ZSTD_c_ldmHashRateLog);

    ADD_INT_PREFIX_MACRO(module, ZSTD_c_contentSizeFlag);
    ADD_INT_PREFIX_MACRO(module, ZSTD_c_checksumFlag);
    ADD_INT_PREFIX_MACRO(module, ZSTD_c_dictIDFlag);

    ADD_INT_PREFIX_MACRO(module, ZSTD_c_nbWorkers);
    ADD_INT_PREFIX_MACRO(module, ZSTD_c_jobSize);
    ADD_INT_PREFIX_MACRO(module, ZSTD_c_overlapLog);

    /* Decompression parameters */
    ADD_INT_PREFIX_MACRO(module, ZSTD_d_windowLogMax);

    /* ZSTD_strategy enum */
    ADD_INT_PREFIX_MACRO(module, ZSTD_fast);
    ADD_INT_PREFIX_MACRO(module, ZSTD_dfast);
    ADD_INT_PREFIX_MACRO(module, ZSTD_greedy);
    ADD_INT_PREFIX_MACRO(module, ZSTD_lazy);
    ADD_INT_PREFIX_MACRO(module, ZSTD_lazy2);
    ADD_INT_PREFIX_MACRO(module, ZSTD_btlazy2);
    ADD_INT_PREFIX_MACRO(module, ZSTD_btopt);
    ADD_INT_PREFIX_MACRO(module, ZSTD_btultra);
    ADD_INT_PREFIX_MACRO(module, ZSTD_btultra2);

    return 0;
}

static inline PyObject *
get_zstd_version_info(void)
{
    const uint32_t ver = ZSTD_versionNumber();
    uint32_t major, minor, release;

    major = ver / 10000;
    minor = (ver / 100) % 100;
    release = ver % 100;

    return Py_BuildValue("III", major, minor, release);
}

static inline int
add_vars_to_module(PyObject *module)
{
    PyObject *obj;

    /* zstd_version, a str. */
    if (PyModule_AddStringConstant(module, "zstd_version",
                                   ZSTD_versionString()) < 0) {
        return -1;
    }

    /* zstd_version_info, a tuple. */
    obj = get_zstd_version_info();
    if (PyModule_AddObject(module, "zstd_version_info", obj) < 0) {
        Py_XDECREF(obj);
        return -1;
    }

    /* Add zstd parameters */
    if (add_parameters(module) < 0) {
        return -1;
    }

    /* _compressionLevel_values: (default, min, max)
       ZSTD_defaultCLevel() was added in zstd v1.5.0 */
    obj = Py_BuildValue("iii",
#if ZSTD_VERSION_NUMBER < 10500
                        ZSTD_CLEVEL_DEFAULT,
#else
                        ZSTD_defaultCLevel(),
#endif
                        ZSTD_minCLevel(),
                        ZSTD_maxCLevel());
    if (PyModule_AddObject(module,
                           "_compressionLevel_values",
                           obj) < 0) {
        Py_XDECREF(obj);
        return -1;
    }

    /* _ZSTD_CStreamSizes */
    obj = Py_BuildValue("II",
                        (uint32_t)ZSTD_CStreamInSize(),
                        (uint32_t)ZSTD_CStreamOutSize());
    if (PyModule_AddObject(module, "_ZSTD_CStreamSizes", obj) < 0) {
        Py_XDECREF(obj);
        return -1;
    }

    /* _ZSTD_DStreamSizes */
    obj = Py_BuildValue("II",
                        (uint32_t)ZSTD_DStreamInSize(),
                        (uint32_t)ZSTD_DStreamOutSize());
    if (PyModule_AddObject(module, "_ZSTD_DStreamSizes", obj) < 0) {
        Py_XDECREF(obj);
        return -1;
    }

    return 0;
}

static inline int
add_constant_to_type(PyTypeObject *type, const char *name, const long value)
{
    PyObject *temp;

    temp = PyLong_FromLong(value);
    if (temp == NULL) {
        return -1;
    }

    if (PyObject_SetAttrString((PyObject*) type, name, temp) < 0) {
        Py_DECREF(temp);
        return -1;
    }
    Py_DECREF(temp);

    return 0;
}

static int
_zstd_exec(PyObject *module)
{
    _zstd_state *state = _get_zstd_state(module);

    /* Add variables to module */
    if (add_vars_to_module(module) < 0) {
        return -1;
    }

    state->error = PyErr_NewExceptionWithDoc("_zstd.ZstdError", "Call to the underlying zstd library failed.", NULL, NULL);
    if (state->error == NULL) {
        return -1;
    }

    if (PyModule_AddType(module, (PyTypeObject *)state->error) < 0) {
        return -1;
    }

    state->zstd_dict_type = (PyTypeObject *)PyType_FromModuleAndSpec(module,
                                                            &zstddict_type_spec, NULL);
    if (state->zstd_dict_type == NULL) {
        return -1;
    }

    if (PyModule_AddType(module, state->zstd_dict_type) < 0) {
        return -1;
    }

    state->zstd_compressor_type = (PyTypeObject *)PyType_FromModuleAndSpec(module,
                                                            &zstd_compressor_type_spec, NULL);
    if (state->zstd_compressor_type == NULL) {
        return -1;
    }

    if (PyModule_AddType(module, state->zstd_compressor_type) < 0) {
        return -1;
    }

    /* Add EndDirective enum to ZstdCompressor */
    if (add_constant_to_type(state->zstd_compressor_type,
                             "CONTINUE",
                             ZSTD_e_continue) < 0) {
        return -1;
    }

    if (add_constant_to_type(state->zstd_compressor_type,
                             "FLUSH_BLOCK",
                             ZSTD_e_flush) < 0) {
        return -1;
    }

    if (add_constant_to_type(state->zstd_compressor_type,
                             "FLUSH_FRAME",
                             ZSTD_e_end) < 0) {
        return -1;
    }

    /*state->zstd_decompressor_type = (PyTypeObject *)PyType_FromModuleAndSpec(module,
                                                         &zstd_decompressor_type_spec, NULL);
    if (state->zstd_decompressor_type == NULL) {
        return -1;
    }

    if (PyModule_AddType(module, state->zstd_decompressor_type) < 0) {
        return -1;
    }*/
    return 0;
}

static PyMethodDef _zstd_methods[] = {
    _ZSTD__GET_CPARAM_BOUNDS_METHODDEF
    _ZSTD__GET_DPARAM_BOUNDS_METHODDEF
    _ZSTD__TRAIN_DICT_METHODDEF
    _ZSTD__FINALIZE_DICT_METHODDEF
    {NULL}
};

static PyModuleDef_Slot _zstd_slots[] = {
    {Py_mod_exec, _zstd_exec},
    {0, NULL}
};

static int
_zstd_traverse(PyObject *module, visitproc visit, void *arg)
{
    _zstd_state *state = _get_zstd_state(module);
    Py_VISIT(state->zstd_compressor_type);
    //Py_VISIT(state->zstd_decompressor_type);
    Py_VISIT(state->zstd_dict_type);
    Py_VISIT(state->error);
    return 0;
}

static int
_zstd_clear(PyObject *module)
{
    _zstd_state *state = _get_zstd_state(module);
    Py_CLEAR(state->zstd_compressor_type);
    //Py_CLEAR(state->zstd_decompressor_type);
    Py_CLEAR(state->zstd_dict_type);
    Py_CLEAR(state->error);
    return 0;
}

static void
_zstd_free(void *module)
{
    _zstd_clear((PyObject *)module);
}

static PyModuleDef _zstdmodule = {
    PyModuleDef_HEAD_INIT,
    .m_name = "_zstd",
    .m_size = sizeof(_zstd_state),
    .m_slots = _zstd_slots,
    .m_methods = _zstd_methods,
    .m_traverse = _zstd_traverse,
    .m_clear = _zstd_clear,
    .m_free = _zstd_free,
};

PyMODINIT_FUNC
PyInit__zstd(void)
{
    return PyModuleDef_Init(&_zstdmodule);
}
#pragma endregion
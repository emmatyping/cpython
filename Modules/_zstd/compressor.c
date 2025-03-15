/*
Low level interface to Meta's zstd library for use in the `zstd` Python library.

Original implementation by Ma Lin with patches by Rogdham.

Refactored for the CPython standard library by Emma Harper Smith.
*/

/* ZstdCompressor class definitions */

/*[clinic input]
module _zstd
class _zstd.ZstdCompressor "ZstdCompressor *" "clinic_state()->ZstdCompressor_type"
[clinic start generated code]*/
/*[clinic end generated code: output=da39a3ee5e6b4b0d input=875bf614798f80cb]*/


#ifndef Py_BUILD_CORE_BUILTIN
#  define Py_BUILD_CORE_MODULE 1
#endif

#include "_zstdmodule.h"

#include "buffer.h"

#include <stddef.h>               // offsetof()

/* -----------------------
    ZstdCompressor code
----------------------- */

int
_PyZstd_set_c_parameters(ZstdCompressor *self, PyObject *level_or_options)
{
    size_t zstd_ret;
    _zstd_state* const _module_state = self->module_state; \
    assert(_module_state != NULL);

    /* Integer compression level */
    if (PyLong_Check(level_or_options)) {
        const int level = PyLong_AsInt(level_or_options);
        if (level == -1 && PyErr_Occurred()) {
            PyErr_SetString(PyExc_ValueError,
                            "Compression level should be 32-bit signed int value.");
            return -1;
        }

        /* Save for generating ZSTD_CDICT */
        self->compression_level = level;

        /* Set compressionLevel to compression context */
        zstd_ret = ZSTD_CCtx_setParameter(self->cctx,
                                          ZSTD_c_compressionLevel,
                                          level);

        /* Check error */
        if (ZSTD_isError(zstd_ret)) {
            set_zstd_error(MODULE_STATE, ERR_SET_C_LEVEL, zstd_ret);
            return -1;
        }
        return 0;
    }

    /* Options dict */
    if (PyDict_Check(level_or_options)) {
        PyObject *key, *value;
        Py_ssize_t pos = 0;

        while (PyDict_Next(level_or_options, &pos, &key, &value)) {
            /* Check key type */
            if (Py_TYPE(key) == MS_MEMBER(DParameter_type)) {
                PyErr_SetString(PyExc_TypeError,
                                "Key of compression option dict should "
                                "NOT be DParameter.");
                return -1;
            }

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
                /* Save for generating ZSTD_CDICT */
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
                set_parameter_error(MODULE_STATE, 1, key_v, value_v);
                return -1;
            }
        }
        return 0;
    }

    /* Wrong type should be unreachable */
    PyErr_SetString(PyExc_RuntimeError,
        "Unreachable!");
    return -1;
}

static void
capsule_free_cdict(PyObject *capsule)
{
    ZSTD_CDict *cdict = PyCapsule_GetPointer(capsule, NULL);
    ZSTD_freeCDict(cdict);
}

ZSTD_CDict *
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
            STATE_FROM_OBJ(self);
            PyErr_SetString(MS_MEMBER(ZstdError),
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

int
_PyZstd_load_c_dict(ZstdCompressor *self, PyObject *dict) {

    size_t zstd_ret;
    _zstd_state* const _module_state = self->module_state; \
    assert(_module_state != NULL);
    ZstdDict *zd;
    int type, ret;

    /* Check ZstdDict */
    ret = PyObject_IsInstance(dict, (PyObject*)MS_MEMBER(ZstdDict_type));
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
                                  (PyObject*)MS_MEMBER(ZstdDict_type));
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
        set_zstd_error(MODULE_STATE, ERR_LOAD_C_DICT, zstd_ret);
        return -1;
    }
    return 0;
}

#define clinic_state() ((_zstd_state*)PyType_GetModuleState(type))
#include "clinic/compressor.c.h"
#undef clinic_state

static PyObject *
_zstd_ZstdCompressor_new(PyTypeObject *type, PyObject *Py_UNUSED(args), PyObject *Py_UNUSED(kwargs))
{
    ZstdCompressor *self;
    self = (ZstdCompressor*)type->tp_alloc(type, 0);
    if (self == NULL) {
        goto error;
    }

    /* Keep this first. Set module state to self. */
    SET_STATE_TO_OBJ(type, self);

    assert(self->dict == NULL);
    assert(self->use_multithread == 0);
    assert(self->compression_level == 0);
    assert(self->inited == 0);

    /* Compression context */
    self->cctx = ZSTD_createCCtx();
    if (self->cctx == NULL) {
        STATE_FROM_OBJ(self);
        PyErr_SetString(MS_MEMBER(ZstdError),
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
ZstdCompressor_dealloc(ZstdCompressor *self)
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

/*[clinic input]
_zstd.ZstdCompressor.__init__

    level: object = None
        The compression level to use, defaults to ZSTD_CLEVEL_DEFAULT.
    options: object = None
        A dict object that contains advanced compression parameters.
    zstd_dict: object = None
        A ZstdDict object, a pre-trained zstd dictionary.

A streaming compressor. Thread-safe at method level.
[clinic start generated code]*/

static int
_zstd_ZstdCompressor___init___impl(ZstdCompressor *self, PyObject *level,
                                   PyObject *options, PyObject *zstd_dict)
/*[clinic end generated code: output=215e6c4342732f96 input=faab5cc262771cf4]*/
{
    /* Only called once */
    if (self->inited) {
        PyErr_SetString(PyExc_RuntimeError, init_twice_msg);
        return -1;
    }
    self->inited = 1;

    if (level != Py_None && options != Py_None) {
        PyErr_SetString(PyExc_RuntimeError, "Only one of level or options should be used.");
        return -1;
    }

    /* Set compressLevel/options to compression context */
    if (level != Py_None) {
        if (_PyZstd_set_c_parameters(self, level) < 0) {
            return -1;
        }
    }

    if (options != Py_None) {
        if (_PyZstd_set_c_parameters(self, options) < 0) {
            return -1;
        }
    }

    /* Load dictionary to compression context */
    if (zstd_dict != Py_None) {
        if (_PyZstd_load_c_dict(self, zstd_dict) < 0) {
            return -1;
        }

        /* Py_INCREF the dict */
        Py_INCREF(zstd_dict);
        self->dict = zstd_dict;
    }

    return 0;
}

PyObject *
compress_impl(ZstdCompressor *self, Py_buffer *data,
                const ZSTD_EndDirective end_directive)
{
    ZSTD_inBuffer in;
    ZSTD_outBuffer out;
    _BlocksOutputBuffer buffer = {.list = NULL};
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
            STATE_FROM_OBJ(self);
            set_zstd_error(MODULE_STATE, ERR_COMPRESS, zstd_ret);
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

static PyObject *
compress_mt_continue_impl(ZstdCompressor *self, Py_buffer *data)
{
    ZSTD_inBuffer in;
    ZSTD_outBuffer out;
    _BlocksOutputBuffer buffer = {.list = NULL};
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
            STATE_FROM_OBJ(self);
            set_zstd_error(MODULE_STATE, ERR_COMPRESS, zstd_ret);
            goto error;
        }

        /* Like compress_impl(), output as much as possible. */
        if (out.pos == out.size) {
            if (_OutputBuffer_Grow(&buffer, &out) < 0) {
                goto error;
            }
        } else if (in.pos == in.size) {
            /* Finished */
            assert(mt_continue_should_break(&in, &out));
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
    mode: int(c_default="ZSTD_e_continue") = ZstdCompressor.CONTINUE
        Can be these 3 values ZstdCompressor.CONTINUE,
        ZstdCompressor.FLUSH_BLOCK, ZstdCompressor.FLUSH_FRAME

Provide data to the compressor object.

Return a chunk of compressed data if possible, or b'' otherwise.
[clinic start generated code]*/

static PyObject *
_zstd_ZstdCompressor_compress_impl(ZstdCompressor *self, Py_buffer *data,
                                   int mode)
/*[clinic end generated code: output=ed7982d1cf7b4f98 input=81f01a11e8a7c89e]*/
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

    return ret;
}

/*[clinic input]
_zstd.ZstdCompressor.flush

    mode: int(c_default="ZSTD_e_end") = ZstdCompressor.FLUSH_FRAME
        Can be these 2 values ZstdCompressor.FLUSH_FRAME,
        ZstdCompressor.FLUSH_BLOCK

Flush any remaining data in internal buffer.

Since zstd data consists of one or more independent frames, the compressor
object can still be used after this method is called.
[clinic start generated code]*/

static PyObject *
_zstd_ZstdCompressor_flush_impl(ZstdCompressor *self, int mode)
/*[clinic end generated code: output=b7cf2c8d64dcf2e3 input=bcf4671042aa4ab4]*/
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

/*[clinic input]
_zstd.ZstdCompressor._set_pledged_input_size

    size: object(subclass_of="&PyLong_Type") = NULL
        Uncompressed content size of a frame, None means "unknown size".
    /

*This is an undocumented method, because it may be used incorrectly.*

Since zstd data consists of one or more independent frames, the compressor
object can still be used after this method is called.

Set uncompressed content size of a frame, the size will be written into the
frame header.
1. If called when (.last_mode != .FLUSH_FRAME), a RuntimeError will be raised.
2. If the actual size doesn't match the value, a ZstdError will be raised, and
   the last compressed chunk is likely to be lost.
3. The size is only valid for one frame, then it restores to "unknown size".
[clinic start generated code]*/

static PyObject *
_zstd_ZstdCompressor__set_pledged_input_size_impl(ZstdCompressor *self,
                                                  PyObject *size)
/*[clinic end generated code: output=2d4bbed8d4e569f5 input=cd419a9321502c2c]*/
{
    uint64_t pledged_size;
    size_t zstd_ret;
    PyObject *ret;

    /* Get size value */
    if (size == NULL) {
        pledged_size = ZSTD_CONTENTSIZE_UNKNOWN;
    } else {
        pledged_size = PyLong_AsUnsignedLongLong(size);
        if (pledged_size == (uint64_t)-1 && PyErr_Occurred()) {
            PyErr_SetString(PyExc_ValueError,
                            "size argument should be 64-bit unsigned integer "
                            "value, or None.");
            return NULL;
        }
    }

    /* Thread-safe code */
    ACQUIRE_LOCK(self);

    /* Check the current mode */
    if (self->last_mode != ZSTD_e_end) {
        PyErr_SetString(PyExc_RuntimeError,
                        "._set_pledged_input_size() method must be called "
                        "when (.last_mode == .FLUSH_FRAME).");
        goto error;
    }

    /* Set pledged content size */
    zstd_ret = ZSTD_CCtx_setPledgedSrcSize(self->cctx, pledged_size);
    if (ZSTD_isError(zstd_ret)) {
        STATE_FROM_OBJ(self);
        set_zstd_error(MODULE_STATE, ERR_SET_PLEDGED_INPUT_SIZE, zstd_ret);
        goto error;
    }

    /* Return None */
    ret = Py_None;
    Py_INCREF(ret);
    goto success;

error:
    ret = NULL;
success:
    RELEASE_LOCK(self);
    return ret;
}

static PyMethodDef ZstdCompressor_methods[] = {
    _ZSTD_ZSTDCOMPRESSOR_COMPRESS_METHODDEF
    _ZSTD_ZSTDCOMPRESSOR_FLUSH_METHODDEF
    _ZSTD_ZSTDCOMPRESSOR__SET_PLEDGED_INPUT_SIZE_METHODDEF

    {0}
};

PyDoc_STRVAR(ZstdCompressor_last_mode_doc,
"The last mode used to this compressor object, its value can be .CONTINUE,\n"
".FLUSH_BLOCK, .FLUSH_FRAME. Initialized to .FLUSH_FRAME.\n\n"
"It can be used to get the current state of a compressor, such as, data flushed,\n"
"a frame ended.");

static PyMemberDef ZstdCompressor_members[] = {
    {"last_mode", Py_T_INT, offsetof(ZstdCompressor, last_mode),
        Py_READONLY, ZstdCompressor_last_mode_doc},
    {0}
};

static PyType_Slot zstdcompressor_slots[] = {
    {Py_tp_new, _zstd_ZstdCompressor_new},
    {Py_tp_dealloc, ZstdCompressor_dealloc},
    {Py_tp_init, _zstd_ZstdCompressor___init__},
    {Py_tp_methods, ZstdCompressor_methods},
    {Py_tp_members, ZstdCompressor_members},
    {Py_tp_doc, (char*)_zstd_ZstdCompressor___init____doc__},
    {0}
};

PyType_Spec zstdcompressor_type_spec = {
    .name = "_zstd.ZstdCompressor",
    .basicsize = sizeof(ZstdCompressor),
    .flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .slots = zstdcompressor_slots,
};

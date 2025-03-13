/*
Low level interface to Meta's zstd library for use in the `zstd` Python library.

Original implementation by Ma Lin with patches by Rogdham.

Refactored for the CPython standard library by Emma Harper Smith.
*/

/* ZstdDecompressor and EndlessZstdDecompressor class definitions */

/*[clinic input]
module _zstd
class _zstd.ZstdDecompressor "ZstdDecompressor *" "clinic_state()->ZstdDecompressor_type"
class _zstd.EndlessZstdDecompressor "ZstdDecompressor *" "clinic_state()->EndlessZstdDecompressor_type"
[clinic start generated code]*/
/*[clinic end generated code: output=da39a3ee5e6b4b0d input=d3ec20d7a04d24b1]*/

#ifndef Py_BUILD_CORE_BUILTIN
#  define Py_BUILD_CORE_MODULE 1
#endif

#include "_zstdmodule.h"

#include "buffer.h"

#include <stddef.h>               // offsetof()

/* -----------------------------
     Decompress implementation
   ----------------------------- */

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
            STATE_FROM_OBJ(self);
            PyErr_SetString(MS_MEMBER(ZstdError),
                            "Failed to create ZSTD_DDict instance from zstd "
                            "dictionary content. Maybe the content is corrupted.");
        }
    }

    /* Don't lose any exception */
    ret = self->d_dict;
    RELEASE_LOCK(self);

    return ret;
}

/* Set decompression parameters to decompression context */
int
_PyZstd_set_d_parameters(ZstdDecompressor *self, PyObject *options)
{
    size_t zstd_ret;
    PyObject *key, *value;
    Py_ssize_t pos;
    STATE_FROM_OBJ(self);

    if (!PyDict_Check(options)) {
        PyErr_SetString(PyExc_TypeError,
                        "options argument should be dict object.");
        return -1;
    }

    pos = 0;
    while (PyDict_Next(options, &pos, &key, &value)) {
        /* Check key type */
        if (Py_TYPE(key) == MS_MEMBER(CParameter_type)) {
            PyErr_SetString(PyExc_TypeError,
                            "Key of decompression options dict should "
                            "NOT be CParameter.");
            return -1;
        }

        /* Both key & value should be 32-bit signed int */
        const int key_v = PyLong_AsInt(key);
        if (key_v == -1 && PyErr_Occurred()) {
            PyErr_SetString(PyExc_ValueError,
                            "Key of options dict should be 32-bit signed integer value.");
            return -1;
        }

        const int value_v = PyLong_AsInt(value);
        if (value_v == -1 && PyErr_Occurred()) {
            PyErr_SetString(PyExc_ValueError,
                            "Value of options dict should be 32-bit signed integer value.");
            return -1;
        }

        /* Set parameter to compression context */
        zstd_ret = ZSTD_DCtx_setParameter(self->dctx, key_v, value_v);

        /* Check error */
        if (ZSTD_isError(zstd_ret)) {
            set_parameter_error(MODULE_STATE, 0, key_v, value_v);
            return -1;
        }
    }
    return 0;
}

/* Load dictionary or prefix to decompression context */
int
_PyZstd_load_d_dict(ZstdDecompressor *self, PyObject *dict)
{
    size_t zstd_ret;
    STATE_FROM_OBJ(self);
    ZstdDict *zd;
    int type, ret;

    /* Check ZstdDict */
    ret = PyObject_IsInstance(dict, (PyObject*)MS_MEMBER(ZstdDict_type));
    if (ret < 0) {
        return -1;
    } else if (ret > 0) {
        /* When decompressing, use digested dictionary by default. */
        zd = (ZstdDict*)dict;
        type = DICT_TYPE_DIGESTED;
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
        /* Get ZSTD_DDict */
        ZSTD_DDict *d_dict = _get_DDict(zd);
        if (d_dict == NULL) {
            return -1;
        }
        /* Reference a prepared dictionary */
        zstd_ret = ZSTD_DCtx_refDDict(self->dctx, d_dict);
    } else if (type == DICT_TYPE_UNDIGESTED) {
        /* Load a dictionary */
        zstd_ret = ZSTD_DCtx_loadDictionary(
                            self->dctx,
                            PyBytes_AS_STRING(zd->dict_content),
                            Py_SIZE(zd->dict_content));
    } else if (type == DICT_TYPE_PREFIX) {
        /* Load a prefix */
        zstd_ret = ZSTD_DCtx_refPrefix(
                            self->dctx,
                            PyBytes_AS_STRING(zd->dict_content),
                            Py_SIZE(zd->dict_content));
    } else {
        /* Impossible code path */
        PyErr_SetString(PyExc_SystemError,
                        "load_d_dict() impossible code path");
        return -1;
    }

    /* Check error */
    if (ZSTD_isError(zstd_ret)) {
        set_zstd_error(MODULE_STATE, ERR_LOAD_D_DICT, zstd_ret);
        return -1;
    }
    return 0;
}

typedef enum {
    TYPE_DECOMPRESSOR,          /* <D>, ZstdDecompressor class */
    TYPE_ENDLESS_DECOMPRESSOR,  /* <E>, EndlessZstdDecompressor class */
} decompress_type;

/* Decompress implementation for <D>, <E>, pseudo code:

        initialize_output_buffer
        while True:
            decompress_data
            set_object_flag   # .eof for <D>, .at_frame_edge for <E>.

            if output_buffer_exhausted:
                if output_buffer_reached_max_length:
                    finish
                grow_output_buffer
            elif input_buffer_exhausted:
                finish

    ZSTD_decompressStream()'s size_t return value:
      - 0 when a frame is completely decoded and fully flushed, zstd's internal
        buffer has no data.
      - An error code, which can be tested using ZSTD_isError().
      - Or any other value > 0, which means there is still some decoding or
        flushing to do to complete current frame.

      Note, decompressing "an empty input" in any case will make it > 0.

    <E> supports multiple frames, has an .at_frame_edge flag, it means both the
    input and output streams are at a frame edge. The flag can be set by this
    statement:

        .at_frame_edge = (zstd_ret == 0) ? 1 : 0

    But if decompressing "an empty input" at "a frame edge", zstd_ret will be
    non-zero, then .at_frame_edge will be wrongly set to false. To solve this
    problem, two AFE checks are needed to ensure that: when at "a frame edge",
    empty input will not be decompressed.

        // AFE check
        if (self->at_frame_edge && in->pos == in->size) {
            finish
        }

    In <E>, if .at_frame_edge is eventually set to true, but input stream has
    unconsumed data (in->pos < in->size), then the outer function
    stream_decompress() will set .at_frame_edge to false. In this case,
    although the output stream is at a frame edge, for the caller, the input
    stream is not at a frame edge, see below diagram. This behavior does not
    affect the next AFE check, since (in->pos < in->size).

    input stream:  --------------|---
                                    ^
    output stream: ====================|
                                       ^
*/
PyObject *
decompress_impl(ZstdDecompressor *self, ZSTD_inBuffer *in,
                const Py_ssize_t max_length,
                const Py_ssize_t initial_size,
                const decompress_type type)
{
    size_t zstd_ret;
    ZSTD_outBuffer out;
    _BlocksOutputBuffer buffer = {.list = NULL};
    PyObject *ret;

    /* The first AFE check for setting .at_frame_edge flag */
    if (type == TYPE_ENDLESS_DECOMPRESSOR) {
        if (self->at_frame_edge && in->pos == in->size) {
            STATE_FROM_OBJ(self);
            ret = MS_MEMBER(empty_bytes);
            Py_INCREF(ret);
            return ret;
        }
    }

    /* Initialize the output buffer */
    if (initial_size >= 0) {
        if (_OutputBuffer_InitWithSize(&buffer, &out, max_length, initial_size) < 0) {
            goto error;
        }
    } else {
        if (_OutputBuffer_InitAndGrow(&buffer, &out, max_length) < 0) {
            goto error;
        }
    }
    assert(out.pos == 0);

    while (1) {
        /* Decompress */
        Py_BEGIN_ALLOW_THREADS
        zstd_ret = ZSTD_decompressStream(self->dctx, &out, in);
        Py_END_ALLOW_THREADS

        /* Check error */
        if (ZSTD_isError(zstd_ret)) {
            STATE_FROM_OBJ(self);
            set_zstd_error(MODULE_STATE, ERR_DECOMPRESS, zstd_ret);
            goto error;
        }

        /* Set .eof/.af_frame_edge flag */
        if (type == TYPE_DECOMPRESSOR) {
            /* ZstdDecompressor class stops when a frame is decompressed */
            if (zstd_ret == 0) {
                self->eof = 1;
                break;
            }
        } else if (type == TYPE_ENDLESS_DECOMPRESSOR) {
            /* EndlessZstdDecompressor class supports multiple frames */
            self->at_frame_edge = (zstd_ret == 0) ? 1 : 0;

            /* The second AFE check for setting .at_frame_edge flag */
            if (self->at_frame_edge && in->pos == in->size) {
                break;
            }
        }

        /* Need to check out before in. Maybe zstd's internal buffer still has
           a few bytes can be output, grow the buffer and continue. */
        if (out.pos == out.size) {
            /* Output buffer exhausted */

            /* Output buffer reached max_length */
            if (_OutputBuffer_ReachedMaxLength(&buffer, &out)) {
                break;
            }

            /* Grow output buffer */
            if (_OutputBuffer_Grow(&buffer, &out) < 0) {
                goto error;
            }
            assert(out.pos == 0);

        } else if (in->pos == in->size) {
            /* Finished */
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

void
decompressor_reset_session(ZstdDecompressor *self,
                           const decompress_type type)
{
    /* Reset variables */
    self->in_begin = 0;
    self->in_end = 0;

    if (type == TYPE_DECOMPRESSOR) {
        Py_CLEAR(self->unused_data);
    }

    /* Reset variables in one operation */
    self->needs_input = 1;
    self->at_frame_edge = 1;
    self->eof = 0;
    self->_unused_char_for_align = 0;

    /* Resetting session never fail */
    ZSTD_DCtx_reset(self->dctx, ZSTD_reset_session_only);
}

PyObject *
stream_decompress(ZstdDecompressor *self, Py_buffer *data, Py_ssize_t max_length,
                  const decompress_type type)
{
    Py_ssize_t initial_buffer_size = -1;
    ZSTD_inBuffer in;
    PyObject *ret = NULL;
    int use_input_buffer;

    /* Thread-safe code */
    ACQUIRE_LOCK(self);

    if (type == TYPE_DECOMPRESSOR) {
        /* Check .eof flag */
        if (self->eof) {
            PyErr_SetString(PyExc_EOFError, "Already at the end of a zstd frame.");
            assert(ret == NULL);
            goto success;
        }
    } else if (type == TYPE_ENDLESS_DECOMPRESSOR) {
        /* Fast path for the first frame */
        if (self->at_frame_edge && self->in_begin == self->in_end) {
            /* Read decompressed size */
            uint64_t decompressed_size = ZSTD_getFrameContentSize(data->buf, data->len);

            /* These two zstd constants always > PY_SSIZE_T_MAX:
                  ZSTD_CONTENTSIZE_UNKNOWN is (0ULL - 1)
                  ZSTD_CONTENTSIZE_ERROR   is (0ULL - 2)

               Use ZSTD_findFrameCompressedSize() to check complete frame,
               prevent allocating too much memory for small input chunk. */

            if (decompressed_size <= (uint64_t) PY_SSIZE_T_MAX &&
                !ZSTD_isError(ZSTD_findFrameCompressedSize(data->buf, data->len)) )
            {
                initial_buffer_size = (Py_ssize_t) decompressed_size;
            }
        }
    }

    /* Prepare input buffer w/wo unconsumed data */
    if (self->in_begin == self->in_end) {
        /* No unconsumed data */
        use_input_buffer = 0;

        in.src = data->buf;
        in.size = data->len;
        in.pos = 0;
    } else if (data->len == 0) {
        /* Has unconsumed data, fast path for b'' */
        assert(self->in_begin < self->in_end);

        use_input_buffer = 1;

        in.src = self->input_buffer + self->in_begin;
        in.size = self->in_end - self->in_begin;
        in.pos = 0;
    } else {
        /* Has unconsumed data */
        use_input_buffer = 1;

        /* Unconsumed data size in input_buffer */
        const size_t used_now = self->in_end - self->in_begin;
        assert(self->in_end > self->in_begin);

        /* Number of bytes we can append to input buffer */
        const size_t avail_now = self->input_buffer_size - self->in_end;
        assert(self->input_buffer_size >= self->in_end);

        /* Number of bytes we can append if we move existing contents to
           beginning of buffer */
        const size_t avail_total = self->input_buffer_size - used_now;
        assert(self->input_buffer_size >= used_now);

        if (avail_total < (size_t) data->len) {
            char *tmp;
            const size_t new_size = used_now + data->len;

            /* Allocate with new size */
            tmp = PyMem_Malloc(new_size);
            if (tmp == NULL) {
                PyErr_NoMemory();
                goto error;
            }

            /* Copy unconsumed data to the beginning of new buffer */
            memcpy(tmp,
                   self->input_buffer + self->in_begin,
                   used_now);

            /* Switch to new buffer */
            PyMem_Free(self->input_buffer);
            self->input_buffer = tmp;
            self->input_buffer_size = new_size;

            /* Set begin & end position */
            self->in_begin = 0;
            self->in_end = used_now;
        } else if (avail_now < (size_t) data->len) {
            /* Move unconsumed data to the beginning.
               Overlap is possible, so use memmove(). */
            memmove(self->input_buffer,
                    self->input_buffer + self->in_begin,
                    used_now);

            /* Set begin & end position */
            self->in_begin = 0;
            self->in_end = used_now;
        }

        /* Copy data to input buffer */
        memcpy(self->input_buffer + self->in_end, data->buf, data->len);
        self->in_end += data->len;

        in.src = self->input_buffer + self->in_begin;
        in.size = used_now + data->len;
        in.pos = 0;
    }
    assert(in.pos == 0);

    /* Decompress */
    ret = decompress_impl(self, &in,
                          max_length, initial_buffer_size,
                          type);
    if (ret == NULL) {
        goto error;
    }

    /* Unconsumed input data */
    if (in.pos == in.size) {
        if (type == TYPE_DECOMPRESSOR) {
            if (Py_SIZE(ret) == max_length || self->eof) {
                self->needs_input = 0;
            } else {
                self->needs_input = 1;
            }
        } else if (type == TYPE_ENDLESS_DECOMPRESSOR) {
            if (Py_SIZE(ret) == max_length && !self->at_frame_edge) {
                self->needs_input = 0;
            } else {
                self->needs_input = 1;
            }
        }

        if (use_input_buffer) {
            /* Clear input_buffer */
            self->in_begin = 0;
            self->in_end = 0;
        }
    } else {
        const size_t data_size = in.size - in.pos;

        self->needs_input = 0;

        if (type == TYPE_ENDLESS_DECOMPRESSOR) {
            self->at_frame_edge = 0;
        }

        if (!use_input_buffer) {
            /* Discard buffer if it's too small
               (resizing it may needlessly copy the current contents) */
            if (self->input_buffer != NULL &&
                self->input_buffer_size < data_size)
            {
                PyMem_Free(self->input_buffer);
                self->input_buffer = NULL;
                self->input_buffer_size = 0;
            }

            /* Allocate if necessary */
            if (self->input_buffer == NULL) {
                self->input_buffer = PyMem_Malloc(data_size);
                if (self->input_buffer == NULL) {
                    PyErr_NoMemory();
                    goto error;
                }
                self->input_buffer_size = data_size;
            }

            /* Copy unconsumed data */
            memcpy(self->input_buffer, (char*)in.src + in.pos, data_size);
            self->in_begin = 0;
            self->in_end = data_size;
        } else {
            /* Use input buffer */
            self->in_begin += in.pos;
        }
    }

    goto success;

error:
    /* Reset decompressor's states/session */
    decompressor_reset_session(self, type);

    Py_CLEAR(ret);
success:
    RELEASE_LOCK(self);

    return ret;
}


/* -------------------------
     ZstdDecompressor code
   ------------------------- */

/*[clinic input]
@classmethod
_zstd.ZstdDecompressor.__new__

    args: object(unused=True) = NULL
    *
    kwargs: object(unused=True) = NULL

[clinic start generated code]*/

static PyObject *
_zstd_ZstdDecompressor_impl(PyTypeObject *type, PyObject *Py_UNUSED(args), PyObject *Py_UNUSED(kwargs))
/*[clinic end generated code: output=72dd15adf87526d8 input=9de9bd28e503ceea]*/
{
    ZstdDecompressor *self;
    self = (ZstdDecompressor*)type->tp_alloc(type, 0);
    if (self == NULL) {
        goto error;
    }

    /* Keep this first. Set module state to self. */
    SET_STATE_TO_OBJ(type, self);

    assert(self->dict == NULL);
    assert(self->input_buffer == NULL);
    assert(self->input_buffer_size == 0);
    assert(self->in_begin == 0);
    assert(self->in_end == 0);
    assert(self->unused_data == NULL);
    assert(self->eof == 0);
    assert(self->inited == 0);

    /* needs_input flag */
    self->needs_input = 1;

    /* at_frame_edge flag */
    self->at_frame_edge = 1;

    /* Decompression context */
    self->dctx = ZSTD_createDCtx();
    if (self->dctx == NULL) {
        STATE_FROM_OBJ(self);
        PyErr_SetString(MS_MEMBER(ZstdError),
                        "Unable to create ZSTD_DCtx instance.");
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
ZstdDecompressor_dealloc(ZstdDecompressor *self)
{
    /* Free decompression context */
    ZSTD_freeDCtx(self->dctx);

    /* Py_XDECREF the dict after free decompression context */
    Py_XDECREF(self->dict);

    /* Free unconsumed input data buffer */
    PyMem_Free(self->input_buffer);

    /* Free unused data */
    Py_XDECREF(self->unused_data);

    /* Free thread lock */
    if (self->lock) {
        PyThread_free_lock(self->lock);
    }

    PyTypeObject *tp = Py_TYPE(self);
    tp->tp_free((PyObject*)self);
    Py_DECREF(tp);
}

/*[clinic input]
_zstd.ZstdDecompressor.__init__

    zstd_dict: object = NULL
        A ZstdDict object, a pre-trained zstd dictionary.
    options: object(subclass_of='&PyDict_Type') = NULL
        A dict object that contains advanced decompression parameters.

A streaming decompressor, it stops after a frame is decompressed.

Thread-safe at method level.
[clinic start generated code]*/

static int
_zstd_ZstdDecompressor___init___impl(ZstdDecompressor *self,
                                     PyObject *zstd_dict, PyObject *options)
/*[clinic end generated code: output=703af2f1ec226642 input=80007f69acd9fd24]*/
{
    /* Only called once */
    if (self->inited) {
        PyErr_SetString(PyExc_RuntimeError, init_twice_msg);
        return -1;
    }
    self->inited = 1;

    /* Load dictionary to decompression context */
    if (zstd_dict != NULL) {
        if (_PyZstd_load_d_dict(self, zstd_dict) < 0) {
            return -1;
        }

        /* Py_INCREF the dict */
        Py_INCREF(zstd_dict);
        self->dict = zstd_dict;
    }

    /* Set option to decompression context */
    if (options != NULL) {
        if (_PyZstd_set_d_parameters(self, options) < 0) {
            return -1;
        }
    }

    return 0;
}

/*[clinic input]
@critical_section
@getter
_zstd.ZstdDecompressor.unused_data

A bytes object of un-consumed input data.

When ZstdDecompressor object stops after a frame is
decompressed, unused input data after the frame. Otherwise this will be b''.
[clinic start generated code]*/

static PyObject *
_zstd_ZstdDecompressor_unused_data_get_impl(ZstdDecompressor *self)
/*[clinic end generated code: output=f3a20940f11b6b09 input=5233800bef00df04]*/
{
    PyObject *ret;

    /* Thread-safe code */
    ACQUIRE_LOCK(self);

    if (!self->eof) {
        STATE_FROM_OBJ(self);
        ret = MS_MEMBER(empty_bytes);
        Py_INCREF(ret);
    } else {
        if (self->unused_data == NULL) {
            self->unused_data = PyBytes_FromStringAndSize(
                                    self->input_buffer + self->in_begin,
                                    self->in_end - self->in_begin);
            ret = self->unused_data;
            Py_XINCREF(ret);
        } else {
            ret = self->unused_data;
            Py_INCREF(ret);
        }
    }

    RELEASE_LOCK(self);

    return ret;
}

/*[clinic input]
_zstd.ZstdDecompressor.decompress


    data: Py_buffer
        A bytes-like object, zstd data to be decompressed.
    max_length: Py_ssize_t = -1
        Maximum size of returned data. When it is negative, the size of
        output buffer is unlimited. When it is nonnegative, returns at
        most max_length bytes of decompressed data.

Decompress data, return a chunk of decompressed data if possible, or b'' otherwise.

It stops after a frame is decompressed.
[clinic start generated code]*/

static PyObject *
_zstd_ZstdDecompressor_decompress_impl(ZstdDecompressor *self,
                                       Py_buffer *data,
                                       Py_ssize_t max_length)
/*[clinic end generated code: output=a4302b3c940dbec6 input=1be6c9b78a599c86]*/
{
    return stream_decompress(self, data, max_length, TYPE_DECOMPRESSOR);
}

/*[clinic input]
_zstd.ZstdDecompressor._reset_session


This is an undocumented method. Reset decompressor's states/session, don't reset parameters and dictionary.
[clinic start generated code]*/

static PyObject *
_zstd_ZstdDecompressor__reset_session_impl(ZstdDecompressor *self)
/*[clinic end generated code: output=f05c0b4ccbaec21a input=d384cfba2db3e71d]*/
{
    /* Thread-safe code */
    ACQUIRE_LOCK(self);
    decompressor_reset_session(self, TYPE_DECOMPRESSOR);
    RELEASE_LOCK(self);

    Py_RETURN_NONE;
}

#define clinic_state() ((_zstd_state*)PyType_GetModuleState(type))
#include "clinic/decompressor.c.h"
#undef clinic_state

static PyMethodDef ZstdDecompressor_methods[] = {
    _ZSTD_ZSTDDECOMPRESSOR_DECOMPRESS_METHODDEF

    _ZSTD_ZSTDDECOMPRESSOR__RESET_SESSION_METHODDEF

    {0}
};

PyDoc_STRVAR(ZstdDecompressor_eof_doc,
"True means the end of the first frame has been reached. If decompress data\n"
"after that, an EOFError exception will be raised.");

PyDoc_STRVAR(ZstdDecompressor_needs_input_doc,
"If the max_length output limit in .decompress() method has been reached, and\n"
"the decompressor has (or may has) unconsumed input data, it will be set to\n"
"False. In this case, pass b'' to .decompress() method may output further data.");

static PyMemberDef ZstdDecompressor_members[] = {
    {"eof", Py_T_BOOL, offsetof(ZstdDecompressor, eof),
    Py_READONLY, ZstdDecompressor_eof_doc},

    {"needs_input", Py_T_BOOL, offsetof(ZstdDecompressor, needs_input),
    Py_READONLY, ZstdDecompressor_needs_input_doc},

    {0}
};

static PyGetSetDef ZstdDecompressor_getset[] = {
    _ZSTD_ZSTDDECOMPRESSOR_UNUSED_DATA_GETSETDEF

    {0}
};

static PyType_Slot ZstdDecompressor_slots[] = {
    {Py_tp_new, _zstd_ZstdDecompressor},
    {Py_tp_dealloc, ZstdDecompressor_dealloc},
    {Py_tp_init, _zstd_ZstdDecompressor___init__},
    {Py_tp_methods, ZstdDecompressor_methods},
    {Py_tp_members, ZstdDecompressor_members},
    {Py_tp_getset, ZstdDecompressor_getset},
    {Py_tp_doc, (char*)_zstd_ZstdDecompressor___init____doc__},
    {0}
};

PyType_Spec ZstdDecompressor_type_spec = {
    .name = "_zstd.ZstdDecompressor",
    .basicsize = sizeof(ZstdDecompressor),
    .flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .slots = ZstdDecompressor_slots,
};

/* -------------------------------
    EndlessZstdDecompressor code
    ------------------------------- */
PyDoc_STRVAR(EndlessZstdDecompressor_doc,
"A streaming decompressor, accepts multiple concatenated frames.\n"
"Thread-safe at method level.\n\n"
"EndlessZstdDecompressor.__init__(self, zstd_dict=None, option=None)\n"
"----\n"
"Initialize an EndlessZstdDecompressor object.\n\n"
"Parameters\n"
"zstd_dict: A ZstdDict object, pre-trained zstd dictionary.\n"
"option:    A dict object that contains advanced decompression parameters.");

/*[clinic input]
_zstd.EndlessZstdDecompressor.decompress

    data: Py_buffer
        A bytes-like object, zstd data to be decompressed.
    max_length: Py_ssize_t = -1
        Maximum size of returned data. When it is negative, the size of
        output buffer is unlimited. When it is nonnegative, returns at
        most max_length bytes of decompressed data.

Decompress data, return a chunk of decompressed data if possible, or b'' otherwise.

It stops after a frame is decompressed.
[clinic start generated code]*/

static PyObject *
_zstd_EndlessZstdDecompressor_decompress_impl(ZstdDecompressor *self,
                                              Py_buffer *data,
                                              Py_ssize_t max_length)
/*[clinic end generated code: output=319c125a794cae78 input=358442e527ffc63d]*/
{
    return stream_decompress(self, data, max_length, TYPE_ENDLESS_DECOMPRESSOR);
}


/*[clinic input]
_zstd.EndlessZstdDecompressor._reset_session


This is an undocumented method. Reset decompressor's states/session, don't reset parameters and dictionary.
[clinic start generated code]*/

static PyObject *
_zstd_EndlessZstdDecompressor__reset_session_impl(ZstdDecompressor *self)
/*[clinic end generated code: output=2ba088001d47ef68 input=1ad40dfd00627bac]*/
{
    /* Thread-safe code */
    ACQUIRE_LOCK(self);
    decompressor_reset_session(self, TYPE_ENDLESS_DECOMPRESSOR);
    RELEASE_LOCK(self);

    Py_RETURN_NONE;
}

static PyMethodDef EndlessZstdDecompressor_methods[] = {
    _ZSTD_ENDLESSZSTDDECOMPRESSOR_DECOMPRESS_METHODDEF

    _ZSTD_ENDLESSZSTDDECOMPRESSOR__RESET_SESSION_METHODDEF

    {0}
};

PyDoc_STRVAR(EndlessZstdDecompressor_at_frame_edge_doc,
"True when both the input and output streams are at a frame edge, means a frame is\n"
"completely decoded and fully flushed, or the decompressor just be initialized.\n\n"
"This flag could be used to check data integrity in some cases.");

static PyMemberDef EndlessZstdDecompressor_members[] = {
    {"at_frame_edge", Py_T_BOOL, offsetof(ZstdDecompressor, at_frame_edge),
    Py_READONLY, EndlessZstdDecompressor_at_frame_edge_doc},

    {"needs_input", Py_T_BOOL, offsetof(ZstdDecompressor, needs_input),
    Py_READONLY, ZstdDecompressor_needs_input_doc},

    {0}
};

static PyType_Slot EndlessZstdDecompressor_slots[] = {
    {Py_tp_new, _zstd_ZstdDecompressor},
    {Py_tp_dealloc, ZstdDecompressor_dealloc},
    {Py_tp_init, _zstd_ZstdDecompressor___init__},
    {Py_tp_methods, EndlessZstdDecompressor_methods},
    {Py_tp_members, EndlessZstdDecompressor_members},
    {Py_tp_doc, (char*)EndlessZstdDecompressor_doc},
    {0}
};

PyType_Spec EndlessZstdDecompressor_type_spec = {
    .name = "_zstd.EndlessZstdDecompressor",
    .basicsize = sizeof(ZstdDecompressor),
    .flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .slots = EndlessZstdDecompressor_slots,
};

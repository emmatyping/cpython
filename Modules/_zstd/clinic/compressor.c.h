/*[clinic input]
preserve
[clinic start generated code]*/

#if defined(Py_BUILD_CORE) && !defined(Py_BUILD_CORE_MODULE)
#  include "pycore_gc.h"          // PyGC_Head
#  include "pycore_runtime.h"     // _Py_ID()
#endif
#include "pycore_modsupport.h"    // _PyArg_UnpackKeywords()

static PyObject *
_zstd_ZstdCompressor_impl(PyTypeObject *type, PyObject *args,
                          PyObject *kwargs);

static PyObject *
_zstd_ZstdCompressor(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
    PyObject *return_value = NULL;
    #if defined(Py_BUILD_CORE) && !defined(Py_BUILD_CORE_MODULE)

    #define NUM_KEYWORDS 2
    static struct {
        PyGC_Head _this_is_not_used;
        PyObject_VAR_HEAD
        PyObject *ob_item[NUM_KEYWORDS];
    } _kwtuple = {
        .ob_base = PyVarObject_HEAD_INIT(&PyTuple_Type, NUM_KEYWORDS)
        .ob_item = { &_Py_ID(args), &_Py_ID(kwargs), },
    };
    #undef NUM_KEYWORDS
    #define KWTUPLE (&_kwtuple.ob_base.ob_base)

    #else  // !Py_BUILD_CORE
    #  define KWTUPLE NULL
    #endif  // !Py_BUILD_CORE

    static const char * const _keywords[] = {"args", "kwargs", NULL};
    static _PyArg_Parser _parser = {
        .keywords = _keywords,
        .fname = "ZstdCompressor",
        .kwtuple = KWTUPLE,
    };
    #undef KWTUPLE
    PyObject *argsbuf[2];
    PyObject * const *fastargs;
    Py_ssize_t nargs = PyTuple_GET_SIZE(args);
    Py_ssize_t noptargs = nargs + (kwargs ? PyDict_GET_SIZE(kwargs) : 0) - 0;
    PyObject *__clinic_args = NULL;
    PyObject *__clinic_kwargs = NULL;

    fastargs = _PyArg_UnpackKeywords(_PyTuple_CAST(args)->ob_item, nargs, kwargs, NULL, &_parser,
            /*minpos*/ 0, /*maxpos*/ 1, /*minkw*/ 0, /*varpos*/ 0, argsbuf);
    if (!fastargs) {
        goto exit;
    }
    if (!noptargs) {
        goto skip_optional_pos;
    }
    if (fastargs[0]) {
        __clinic_args = fastargs[0];
        if (!--noptargs) {
            goto skip_optional_pos;
        }
    }
skip_optional_pos:
    if (!noptargs) {
        goto skip_optional_kwonly;
    }
    __clinic_kwargs = fastargs[1];
skip_optional_kwonly:
    return_value = _zstd_ZstdCompressor_impl(type, __clinic_args, __clinic_kwargs);

exit:
    return return_value;
}

PyDoc_STRVAR(_zstd_ZstdCompressor___init____doc__,
"ZstdCompressor(level=<unrepresentable>, options=<unrepresentable>,\n"
"               zstd_dict=<unrepresentable>)\n"
"--\n"
"\n"
"A streaming compressor. Thread-safe at method level.\n"
"\n"
"  level\n"
"    The compression level to use, defaults to ZSTD_CLEVEL_DEFAULT.\n"
"  options\n"
"    Advanced compression parameters.\n"
"  zstd_dict\n"
"    A ZstdDict object, a pre-trained zstd dictionary.");

static int
_zstd_ZstdCompressor___init___impl(ZstdCompressor *self, PyObject *level,
                                   PyObject *options, PyObject *zstd_dict);

static int
_zstd_ZstdCompressor___init__(PyObject *self, PyObject *args, PyObject *kwargs)
{
    int return_value = -1;
    #if defined(Py_BUILD_CORE) && !defined(Py_BUILD_CORE_MODULE)

    #define NUM_KEYWORDS 3
    static struct {
        PyGC_Head _this_is_not_used;
        PyObject_VAR_HEAD
        PyObject *ob_item[NUM_KEYWORDS];
    } _kwtuple = {
        .ob_base = PyVarObject_HEAD_INIT(&PyTuple_Type, NUM_KEYWORDS)
        .ob_item = { &_Py_ID(level), &_Py_ID(options), &_Py_ID(zstd_dict), },
    };
    #undef NUM_KEYWORDS
    #define KWTUPLE (&_kwtuple.ob_base.ob_base)

    #else  // !Py_BUILD_CORE
    #  define KWTUPLE NULL
    #endif  // !Py_BUILD_CORE

    static const char * const _keywords[] = {"level", "options", "zstd_dict", NULL};
    static _PyArg_Parser _parser = {
        .keywords = _keywords,
        .fname = "ZstdCompressor",
        .kwtuple = KWTUPLE,
    };
    #undef KWTUPLE
    PyObject *argsbuf[3];
    PyObject * const *fastargs;
    Py_ssize_t nargs = PyTuple_GET_SIZE(args);
    Py_ssize_t noptargs = nargs + (kwargs ? PyDict_GET_SIZE(kwargs) : 0) - 0;
    PyObject *level = NULL;
    PyObject *options = NULL;
    PyObject *zstd_dict = NULL;

    fastargs = _PyArg_UnpackKeywords(_PyTuple_CAST(args)->ob_item, nargs, kwargs, NULL, &_parser,
            /*minpos*/ 0, /*maxpos*/ 3, /*minkw*/ 0, /*varpos*/ 0, argsbuf);
    if (!fastargs) {
        goto exit;
    }
    if (!noptargs) {
        goto skip_optional_pos;
    }
    if (fastargs[0]) {
        if (!PyLong_Check(fastargs[0])) {
            _PyArg_BadArgument("ZstdCompressor", "argument 'level'", "int", fastargs[0]);
            goto exit;
        }
        level = fastargs[0];
        if (!--noptargs) {
            goto skip_optional_pos;
        }
    }
    if (fastargs[1]) {
        if (!PyDict_Check(fastargs[1])) {
            _PyArg_BadArgument("ZstdCompressor", "argument 'options'", "dict", fastargs[1]);
            goto exit;
        }
        options = fastargs[1];
        if (!--noptargs) {
            goto skip_optional_pos;
        }
    }
    zstd_dict = fastargs[2];
skip_optional_pos:
    return_value = _zstd_ZstdCompressor___init___impl((ZstdCompressor *)self, level, options, zstd_dict);

exit:
    return return_value;
}

PyDoc_STRVAR(_zstd_ZstdCompressor_compress__doc__,
"compress($self, data, mode=ZstdCompressor.CONTINUE, /)\n"
"--\n"
"\n"
"Provide data to the compressor object.\n"
"\n"
"  mode\n"
"    Can be these 3 values ZstdCompressor.CONTINUE,\n"
"    ZstdCompressor.FLUSH_BLOCK, ZstdCompressor.FLUSH_FRAME\n"
"\n"
"Return a chunk of compressed data if possible, or b\'\' otherwise.");

#define _ZSTD_ZSTDCOMPRESSOR_COMPRESS_METHODDEF    \
    {"compress", _PyCFunction_CAST(_zstd_ZstdCompressor_compress), METH_FASTCALL, _zstd_ZstdCompressor_compress__doc__},

static PyObject *
_zstd_ZstdCompressor_compress_impl(ZstdCompressor *self, Py_buffer *data,
                                   int mode);

static PyObject *
_zstd_ZstdCompressor_compress(PyObject *self, PyObject *const *args, Py_ssize_t nargs)
{
    PyObject *return_value = NULL;
    Py_buffer data = {NULL, NULL};
    int mode = ZSTD_e_continue;

    if (!_PyArg_CheckPositional("compress", nargs, 1, 2)) {
        goto exit;
    }
    if (PyObject_GetBuffer(args[0], &data, PyBUF_SIMPLE) != 0) {
        goto exit;
    }
    if (nargs < 2) {
        goto skip_optional;
    }
    mode = PyLong_AsInt(args[1]);
    if (mode == -1 && PyErr_Occurred()) {
        goto exit;
    }
skip_optional:
    return_value = _zstd_ZstdCompressor_compress_impl((ZstdCompressor *)self, &data, mode);

exit:
    /* Cleanup for data */
    if (data.obj) {
       PyBuffer_Release(&data);
    }

    return return_value;
}

PyDoc_STRVAR(_zstd_ZstdCompressor_flush__doc__,
"flush($self, mode=ZstdCompressor.FLUSH_FRAME, /)\n"
"--\n"
"\n"
"Flush any remaining data in internal buffer.\n"
"\n"
"  mode\n"
"    Can be these 2 values ZstdCompressor.FLUSH_FRAME,\n"
"    ZstdCompressor.FLUSH_BLOCK\n"
"\n"
"Since zstd data consists of one or more independent frames, the compressor\n"
"object can still be used after this method is called.");

#define _ZSTD_ZSTDCOMPRESSOR_FLUSH_METHODDEF    \
    {"flush", _PyCFunction_CAST(_zstd_ZstdCompressor_flush), METH_FASTCALL, _zstd_ZstdCompressor_flush__doc__},

static PyObject *
_zstd_ZstdCompressor_flush_impl(ZstdCompressor *self, int mode);

static PyObject *
_zstd_ZstdCompressor_flush(PyObject *self, PyObject *const *args, Py_ssize_t nargs)
{
    PyObject *return_value = NULL;
    int mode = ZSTD_e_end;

    if (!_PyArg_CheckPositional("flush", nargs, 0, 1)) {
        goto exit;
    }
    if (nargs < 1) {
        goto skip_optional;
    }
    mode = PyLong_AsInt(args[0]);
    if (mode == -1 && PyErr_Occurred()) {
        goto exit;
    }
skip_optional:
    return_value = _zstd_ZstdCompressor_flush_impl((ZstdCompressor *)self, mode);

exit:
    return return_value;
}

PyDoc_STRVAR(_zstd_ZstdCompressor__set_pledged_input_size__doc__,
"_set_pledged_input_size($self, size=<unrepresentable>, /)\n"
"--\n"
"\n"
"*This is an undocumented method, because it may be used incorrectly.*\n"
"\n"
"  size\n"
"    Uncompressed content size of a frame, None means \"unknown size\".\n"
"\n"
"Since zstd data consists of one or more independent frames, the compressor\n"
"object can still be used after this method is called.\n"
"\n"
"Set uncompressed content size of a frame, the size will be written into the\n"
"frame header.\n"
"1. If called when (.last_mode != .FLUSH_FRAME), a RuntimeError will be raised.\n"
"2. If the actual size doesn\'t match the value, a ZstdError will be raised, and\n"
"   the last compressed chunk is likely to be lost.\n"
"3. The size is only valid for one frame, then it restores to \"unknown size\".");

#define _ZSTD_ZSTDCOMPRESSOR__SET_PLEDGED_INPUT_SIZE_METHODDEF    \
    {"_set_pledged_input_size", _PyCFunction_CAST(_zstd_ZstdCompressor__set_pledged_input_size), METH_FASTCALL, _zstd_ZstdCompressor__set_pledged_input_size__doc__},

static PyObject *
_zstd_ZstdCompressor__set_pledged_input_size_impl(ZstdCompressor *self,
                                                  PyObject *size);

static PyObject *
_zstd_ZstdCompressor__set_pledged_input_size(PyObject *self, PyObject *const *args, Py_ssize_t nargs)
{
    PyObject *return_value = NULL;
    PyObject *size = NULL;

    if (!_PyArg_CheckPositional("_set_pledged_input_size", nargs, 0, 1)) {
        goto exit;
    }
    if (nargs < 1) {
        goto skip_optional;
    }
    if (!PyLong_Check(args[0])) {
        _PyArg_BadArgument("_set_pledged_input_size", "argument 1", "int", args[0]);
        goto exit;
    }
    size = args[0];
skip_optional:
    return_value = _zstd_ZstdCompressor__set_pledged_input_size_impl((ZstdCompressor *)self, size);

exit:
    return return_value;
}
/*[clinic end generated code: output=c8f1e391d426819e input=a9049054013a1b77]*/

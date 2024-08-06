/*[clinic input]
preserve
[clinic start generated code]*/

#if defined(Py_BUILD_CORE) && !defined(Py_BUILD_CORE_MODULE)
#  include "pycore_gc.h"          // PyGC_Head
#  include "pycore_runtime.h"     // _Py_ID()
#endif
#include "pycore_abstract.h"      // _PyNumber_Index()
#include "pycore_modsupport.h"    // _PyArg_CheckPositional()

PyDoc_STRVAR(_zstd_ZstdCompressor_compress__doc__,
"compress($self, data, mode, /)\n"
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
_zstd_ZstdCompressor_compress(ZstdCompressor *self, PyObject *const *args, Py_ssize_t nargs)
{
    PyObject *return_value = NULL;
    Py_buffer data = {NULL, NULL};
    int mode;

    if (!_PyArg_CheckPositional("compress", nargs, 2, 2)) {
        goto exit;
    }
    if (PyObject_GetBuffer(args[0], &data, PyBUF_SIMPLE) != 0) {
        goto exit;
    }
    mode = PyLong_AsInt(args[1]);
    if (mode == -1 && PyErr_Occurred()) {
        goto exit;
    }
    return_value = _zstd_ZstdCompressor_compress_impl(self, &data, mode);

exit:
    /* Cleanup for data */
    if (data.obj) {
       PyBuffer_Release(&data);
    }

    return return_value;
}

PyDoc_STRVAR(_zstd_ZstdCompressor_flush__doc__,
"flush($self, mode, /)\n"
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
    {"flush", (PyCFunction)_zstd_ZstdCompressor_flush, METH_O, _zstd_ZstdCompressor_flush__doc__},

static PyObject *
_zstd_ZstdCompressor_flush_impl(ZstdCompressor *self, int mode);

static PyObject *
_zstd_ZstdCompressor_flush(ZstdCompressor *self, PyObject *arg)
{
    PyObject *return_value = NULL;
    int mode;

    mode = PyLong_AsInt(arg);
    if (mode == -1 && PyErr_Occurred()) {
        goto exit;
    }
    return_value = _zstd_ZstdCompressor_flush_impl(self, mode);

exit:
    return return_value;
}

PyDoc_STRVAR(_zstd_ZstdCompressor___init____doc__,
"ZstdCompressor(compression_level=0, options=<unrepresentable>,\n"
"               zstd_dict=<unrepresentable>)\n"
"--\n"
"\n"
"Create a streaming compressor object for compressing data incrementally.\n"
"\n"
"  compression_level\n"
"    Compression level to use.\n"
"  options\n"
"    A mapping of advanced compression parameters.\n"
"  zstd_dict\n"
"    A ZstdDict object, pre-trained zstd dictionary.\n"
"\n"
"For one-shot compression, use the compress() function instead.");

static int
_zstd_ZstdCompressor___init___impl(ZstdCompressor *self,
                                   int compression_level, PyObject *options,
                                   PyObject *zstd_dict);

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
        .ob_item = { &_Py_ID(compression_level), &_Py_ID(options), &_Py_ID(zstd_dict), },
    };
    #undef NUM_KEYWORDS
    #define KWTUPLE (&_kwtuple.ob_base.ob_base)

    #else  // !Py_BUILD_CORE
    #  define KWTUPLE NULL
    #endif  // !Py_BUILD_CORE

    static const char * const _keywords[] = {"compression_level", "options", "zstd_dict", NULL};
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
    int compression_level = 0;
    PyObject *options = NULL;
    PyObject *zstd_dict = NULL;

    fastargs = _PyArg_UnpackKeywords(_PyTuple_CAST(args)->ob_item, nargs, kwargs, NULL, &_parser, 0, 3, 0, argsbuf);
    if (!fastargs) {
        goto exit;
    }
    if (!noptargs) {
        goto skip_optional_pos;
    }
    if (fastargs[0]) {
        compression_level = PyLong_AsInt(fastargs[0]);
        if (compression_level == -1 && PyErr_Occurred()) {
            goto exit;
        }
        if (!--noptargs) {
            goto skip_optional_pos;
        }
    }
    if (fastargs[1]) {
        options = fastargs[1];
        if (!--noptargs) {
            goto skip_optional_pos;
        }
    }
    zstd_dict = fastargs[2];
skip_optional_pos:
    return_value = _zstd_ZstdCompressor___init___impl((ZstdCompressor *)self, compression_level, options, zstd_dict);

exit:
    return return_value;
}

PyDoc_STRVAR(_zstd_ZstdDict___init____doc__,
"ZstdDict(dict_content, is_raw=False)\n"
"--\n"
"\n"
"Zstd dictionary, used for compression/decompression.\n"
"\n"
"  dict_content\n"
"    A bytes-like object, dictionary\'s content.\n"
"  is_raw\n"
"    This parameter is for advanced users. True means dict_content\n"
"    argument is a \"raw content\" dictionary, free of any format\n"
"    restriction. False means dict_content argument is an ordinary\n"
"    zstd dictionary, was created by zstd functions, follow a\n"
"    specified format.");

static int
_zstd_ZstdDict___init___impl(ZstdDict *self, PyObject *dict_content,
                             int is_raw);

static int
_zstd_ZstdDict___init__(PyObject *self, PyObject *args, PyObject *kwargs)
{
    int return_value = -1;
    #if defined(Py_BUILD_CORE) && !defined(Py_BUILD_CORE_MODULE)

    #define NUM_KEYWORDS 2
    static struct {
        PyGC_Head _this_is_not_used;
        PyObject_VAR_HEAD
        PyObject *ob_item[NUM_KEYWORDS];
    } _kwtuple = {
        .ob_base = PyVarObject_HEAD_INIT(&PyTuple_Type, NUM_KEYWORDS)
        .ob_item = { &_Py_ID(dict_content), &_Py_ID(is_raw), },
    };
    #undef NUM_KEYWORDS
    #define KWTUPLE (&_kwtuple.ob_base.ob_base)

    #else  // !Py_BUILD_CORE
    #  define KWTUPLE NULL
    #endif  // !Py_BUILD_CORE

    static const char * const _keywords[] = {"dict_content", "is_raw", NULL};
    static _PyArg_Parser _parser = {
        .keywords = _keywords,
        .fname = "ZstdDict",
        .kwtuple = KWTUPLE,
    };
    #undef KWTUPLE
    PyObject *argsbuf[2];
    PyObject * const *fastargs;
    Py_ssize_t nargs = PyTuple_GET_SIZE(args);
    Py_ssize_t noptargs = nargs + (kwargs ? PyDict_GET_SIZE(kwargs) : 0) - 1;
    PyObject *dict_content;
    int is_raw = 0;

    fastargs = _PyArg_UnpackKeywords(_PyTuple_CAST(args)->ob_item, nargs, kwargs, NULL, &_parser, 1, 2, 0, argsbuf);
    if (!fastargs) {
        goto exit;
    }
    dict_content = fastargs[0];
    if (!noptargs) {
        goto skip_optional_pos;
    }
    is_raw = PyObject_IsTrue(fastargs[1]);
    if (is_raw < 0) {
        goto exit;
    }
skip_optional_pos:
    return_value = _zstd_ZstdDict___init___impl((ZstdDict *)self, dict_content, is_raw);

exit:
    return return_value;
}

PyDoc_STRVAR(_zstd__train_dict__doc__,
"_train_dict($module, samples_bytes, samples_size_list, dict_size, /)\n"
"--\n"
"\n"
"Internal function, train a zstd dictionary.\n"
"\n"
"  samples_bytes\n"
"    Concatenation of samples.\n"
"  samples_size_list\n"
"    List of samples\' sizes.\n"
"  dict_size\n"
"    The size of the dictionary.");

#define _ZSTD__TRAIN_DICT_METHODDEF    \
    {"_train_dict", _PyCFunction_CAST(_zstd__train_dict), METH_FASTCALL, _zstd__train_dict__doc__},

static PyObject *
_zstd__train_dict_impl(PyObject *module, PyBytesObject *samples_bytes,
                       PyObject *samples_size_list, Py_ssize_t dict_size);

static PyObject *
_zstd__train_dict(PyObject *module, PyObject *const *args, Py_ssize_t nargs)
{
    PyObject *return_value = NULL;
    PyBytesObject *samples_bytes;
    PyObject *samples_size_list;
    Py_ssize_t dict_size;

    if (!_PyArg_CheckPositional("_train_dict", nargs, 3, 3)) {
        goto exit;
    }
    if (!PyBytes_Check(args[0])) {
        _PyArg_BadArgument("_train_dict", "argument 1", "bytes", args[0]);
        goto exit;
    }
    samples_bytes = (PyBytesObject *)args[0];
    if (!PyList_Check(args[1])) {
        _PyArg_BadArgument("_train_dict", "argument 2", "list", args[1]);
        goto exit;
    }
    samples_size_list = args[1];
    {
        Py_ssize_t ival = -1;
        PyObject *iobj = _PyNumber_Index(args[2]);
        if (iobj != NULL) {
            ival = PyLong_AsSsize_t(iobj);
            Py_DECREF(iobj);
        }
        if (ival == -1 && PyErr_Occurred()) {
            goto exit;
        }
        dict_size = ival;
    }
    return_value = _zstd__train_dict_impl(module, samples_bytes, samples_size_list, dict_size);

exit:
    return return_value;
}

PyDoc_STRVAR(_zstd__finalize_dict__doc__,
"_finalize_dict($module, custom_dict_bytes, samples_bytes,\n"
"               samples_size_list, dict_size, compression_level, /)\n"
"--\n"
"\n"
"Internal function, finalize a zstd dictionary.\n"
"\n"
"  custom_dict_bytes\n"
"    Custom dictionary content.\n"
"  samples_bytes\n"
"    Concatenation of samples.\n"
"  samples_size_list\n"
"    List of samples\' sizes.\n"
"  dict_size\n"
"    The size of the dictionary.\n"
"  compression_level\n"
"    Optimize for a specific zstd compression level, 0 means default.");

#define _ZSTD__FINALIZE_DICT_METHODDEF    \
    {"_finalize_dict", _PyCFunction_CAST(_zstd__finalize_dict), METH_FASTCALL, _zstd__finalize_dict__doc__},

static PyObject *
_zstd__finalize_dict_impl(PyObject *module, PyBytesObject *custom_dict_bytes,
                          PyBytesObject *samples_bytes,
                          PyObject *samples_size_list, Py_ssize_t dict_size,
                          int compression_level);

static PyObject *
_zstd__finalize_dict(PyObject *module, PyObject *const *args, Py_ssize_t nargs)
{
    PyObject *return_value = NULL;
    PyBytesObject *custom_dict_bytes;
    PyBytesObject *samples_bytes;
    PyObject *samples_size_list;
    Py_ssize_t dict_size;
    int compression_level;

    if (!_PyArg_CheckPositional("_finalize_dict", nargs, 5, 5)) {
        goto exit;
    }
    if (!PyBytes_Check(args[0])) {
        _PyArg_BadArgument("_finalize_dict", "argument 1", "bytes", args[0]);
        goto exit;
    }
    custom_dict_bytes = (PyBytesObject *)args[0];
    if (!PyBytes_Check(args[1])) {
        _PyArg_BadArgument("_finalize_dict", "argument 2", "bytes", args[1]);
        goto exit;
    }
    samples_bytes = (PyBytesObject *)args[1];
    if (!PyList_Check(args[2])) {
        _PyArg_BadArgument("_finalize_dict", "argument 3", "list", args[2]);
        goto exit;
    }
    samples_size_list = args[2];
    {
        Py_ssize_t ival = -1;
        PyObject *iobj = _PyNumber_Index(args[3]);
        if (iobj != NULL) {
            ival = PyLong_AsSsize_t(iobj);
            Py_DECREF(iobj);
        }
        if (ival == -1 && PyErr_Occurred()) {
            goto exit;
        }
        dict_size = ival;
    }
    compression_level = PyLong_AsInt(args[4]);
    if (compression_level == -1 && PyErr_Occurred()) {
        goto exit;
    }
    return_value = _zstd__finalize_dict_impl(module, custom_dict_bytes, samples_bytes, samples_size_list, dict_size, compression_level);

exit:
    return return_value;
}

PyDoc_STRVAR(_zstd__get_cparam_bounds__doc__,
"_get_cparam_bounds($module, parameter, /)\n"
"--\n"
"\n"
"Get CParameter bounds.");

#define _ZSTD__GET_CPARAM_BOUNDS_METHODDEF    \
    {"_get_cparam_bounds", (PyCFunction)_zstd__get_cparam_bounds, METH_O, _zstd__get_cparam_bounds__doc__},

static PyObject *
_zstd__get_cparam_bounds_impl(PyObject *module, int parameter);

static PyObject *
_zstd__get_cparam_bounds(PyObject *module, PyObject *arg)
{
    PyObject *return_value = NULL;
    int parameter;

    parameter = PyLong_AsInt(arg);
    if (parameter == -1 && PyErr_Occurred()) {
        goto exit;
    }
    return_value = _zstd__get_cparam_bounds_impl(module, parameter);

exit:
    return return_value;
}

PyDoc_STRVAR(_zstd__get_dparam_bounds__doc__,
"_get_dparam_bounds($module, parameter, /)\n"
"--\n"
"\n"
"Get DParameter bounds.");

#define _ZSTD__GET_DPARAM_BOUNDS_METHODDEF    \
    {"_get_dparam_bounds", (PyCFunction)_zstd__get_dparam_bounds, METH_O, _zstd__get_dparam_bounds__doc__},

static PyObject *
_zstd__get_dparam_bounds_impl(PyObject *module, int parameter);

static PyObject *
_zstd__get_dparam_bounds(PyObject *module, PyObject *arg)
{
    PyObject *return_value = NULL;
    int parameter;

    parameter = PyLong_AsInt(arg);
    if (parameter == -1 && PyErr_Occurred()) {
        goto exit;
    }
    return_value = _zstd__get_dparam_bounds_impl(module, parameter);

exit:
    return return_value;
}
/*[clinic end generated code: output=858e7791c708b269 input=a9049054013a1b77]*/

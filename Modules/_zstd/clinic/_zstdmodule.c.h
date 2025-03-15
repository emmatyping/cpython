/*[clinic input]
preserve
[clinic start generated code]*/

#if defined(Py_BUILD_CORE) && !defined(Py_BUILD_CORE_MODULE)
#  include "pycore_gc.h"          // PyGC_Head
#  include "pycore_runtime.h"     // _Py_ID()
#endif
#include "pycore_abstract.h"      // _PyNumber_Index()
#include "pycore_modsupport.h"    // _PyArg_CheckPositional()

PyDoc_STRVAR(_zstd__train_dict__doc__,
"_train_dict($module, samples_bytes, samples_size_list, dict_size, /)\n"
"--\n"
"\n"
"Internal function, train a zstd dictionary on sample data.\n"
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

PyDoc_STRVAR(_zstd__get_param_bounds__doc__,
"_get_param_bounds($module, /, is_compress, parameter)\n"
"--\n"
"\n"
"Internal function, get CParameter/DParameter bounds.\n"
"\n"
"  is_compress\n"
"    True for CParameter, False for DParameter.\n"
"  parameter\n"
"    The parameter to get bounds.");

#define _ZSTD__GET_PARAM_BOUNDS_METHODDEF    \
    {"_get_param_bounds", _PyCFunction_CAST(_zstd__get_param_bounds), METH_FASTCALL|METH_KEYWORDS, _zstd__get_param_bounds__doc__},

static PyObject *
_zstd__get_param_bounds_impl(PyObject *module, int is_compress,
                             int parameter);

static PyObject *
_zstd__get_param_bounds(PyObject *module, PyObject *const *args, Py_ssize_t nargs, PyObject *kwnames)
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
        .ob_item = { &_Py_ID(is_compress), &_Py_ID(parameter), },
    };
    #undef NUM_KEYWORDS
    #define KWTUPLE (&_kwtuple.ob_base.ob_base)

    #else  // !Py_BUILD_CORE
    #  define KWTUPLE NULL
    #endif  // !Py_BUILD_CORE

    static const char * const _keywords[] = {"is_compress", "parameter", NULL};
    static _PyArg_Parser _parser = {
        .keywords = _keywords,
        .fname = "_get_param_bounds",
        .kwtuple = KWTUPLE,
    };
    #undef KWTUPLE
    PyObject *argsbuf[2];
    int is_compress;
    int parameter;

    args = _PyArg_UnpackKeywords(args, nargs, NULL, kwnames, &_parser,
            /*minpos*/ 2, /*maxpos*/ 2, /*minkw*/ 0, /*varpos*/ 0, argsbuf);
    if (!args) {
        goto exit;
    }
    is_compress = PyObject_IsTrue(args[0]);
    if (is_compress < 0) {
        goto exit;
    }
    parameter = PyLong_AsInt(args[1]);
    if (parameter == -1 && PyErr_Occurred()) {
        goto exit;
    }
    return_value = _zstd__get_param_bounds_impl(module, is_compress, parameter);

exit:
    return return_value;
}

PyDoc_STRVAR(_zstd__get_frame_size__doc__,
"_get_frame_size($module, /, frame_buffer)\n"
"--\n"
"\n"
"Get the size of a zstd frame, including frame header and 4-byte checksum if it has one.\n"
"\n"
"  frame_buffer\n"
"    A bytes-like object, it should start from the beginning of a frame,\n"
"    and contains at least one complete frame.\n"
"\n"
"It will iterate all blocks\' header within a frame, to accumulate the frame size.");

#define _ZSTD__GET_FRAME_SIZE_METHODDEF    \
    {"_get_frame_size", _PyCFunction_CAST(_zstd__get_frame_size), METH_FASTCALL|METH_KEYWORDS, _zstd__get_frame_size__doc__},

static PyObject *
_zstd__get_frame_size_impl(PyObject *module, Py_buffer *frame_buffer);

static PyObject *
_zstd__get_frame_size(PyObject *module, PyObject *const *args, Py_ssize_t nargs, PyObject *kwnames)
{
    PyObject *return_value = NULL;
    #if defined(Py_BUILD_CORE) && !defined(Py_BUILD_CORE_MODULE)

    #define NUM_KEYWORDS 1
    static struct {
        PyGC_Head _this_is_not_used;
        PyObject_VAR_HEAD
        PyObject *ob_item[NUM_KEYWORDS];
    } _kwtuple = {
        .ob_base = PyVarObject_HEAD_INIT(&PyTuple_Type, NUM_KEYWORDS)
        .ob_item = { &_Py_ID(frame_buffer), },
    };
    #undef NUM_KEYWORDS
    #define KWTUPLE (&_kwtuple.ob_base.ob_base)

    #else  // !Py_BUILD_CORE
    #  define KWTUPLE NULL
    #endif  // !Py_BUILD_CORE

    static const char * const _keywords[] = {"frame_buffer", NULL};
    static _PyArg_Parser _parser = {
        .keywords = _keywords,
        .fname = "_get_frame_size",
        .kwtuple = KWTUPLE,
    };
    #undef KWTUPLE
    PyObject *argsbuf[1];
    Py_buffer frame_buffer = {NULL, NULL};

    args = _PyArg_UnpackKeywords(args, nargs, NULL, kwnames, &_parser,
            /*minpos*/ 1, /*maxpos*/ 1, /*minkw*/ 0, /*varpos*/ 0, argsbuf);
    if (!args) {
        goto exit;
    }
    if (PyObject_GetBuffer(args[0], &frame_buffer, PyBUF_SIMPLE) != 0) {
        goto exit;
    }
    return_value = _zstd__get_frame_size_impl(module, &frame_buffer);

exit:
    /* Cleanup for frame_buffer */
    if (frame_buffer.obj) {
       PyBuffer_Release(&frame_buffer);
    }

    return return_value;
}

PyDoc_STRVAR(_zstd__get_frame_info__doc__,
"_get_frame_info($module, /, frame_buffer)\n"
"--\n"
"\n"
"Internal function, get zstd frame infomation from a frame header.\n"
"\n"
"  frame_buffer\n"
"    A bytes-like object, containing the header of a zstd frame.");

#define _ZSTD__GET_FRAME_INFO_METHODDEF    \
    {"_get_frame_info", _PyCFunction_CAST(_zstd__get_frame_info), METH_FASTCALL|METH_KEYWORDS, _zstd__get_frame_info__doc__},

static PyObject *
_zstd__get_frame_info_impl(PyObject *module, Py_buffer *frame_buffer);

static PyObject *
_zstd__get_frame_info(PyObject *module, PyObject *const *args, Py_ssize_t nargs, PyObject *kwnames)
{
    PyObject *return_value = NULL;
    #if defined(Py_BUILD_CORE) && !defined(Py_BUILD_CORE_MODULE)

    #define NUM_KEYWORDS 1
    static struct {
        PyGC_Head _this_is_not_used;
        PyObject_VAR_HEAD
        PyObject *ob_item[NUM_KEYWORDS];
    } _kwtuple = {
        .ob_base = PyVarObject_HEAD_INIT(&PyTuple_Type, NUM_KEYWORDS)
        .ob_item = { &_Py_ID(frame_buffer), },
    };
    #undef NUM_KEYWORDS
    #define KWTUPLE (&_kwtuple.ob_base.ob_base)

    #else  // !Py_BUILD_CORE
    #  define KWTUPLE NULL
    #endif  // !Py_BUILD_CORE

    static const char * const _keywords[] = {"frame_buffer", NULL};
    static _PyArg_Parser _parser = {
        .keywords = _keywords,
        .fname = "_get_frame_info",
        .kwtuple = KWTUPLE,
    };
    #undef KWTUPLE
    PyObject *argsbuf[1];
    Py_buffer frame_buffer = {NULL, NULL};

    args = _PyArg_UnpackKeywords(args, nargs, NULL, kwnames, &_parser,
            /*minpos*/ 1, /*maxpos*/ 1, /*minkw*/ 0, /*varpos*/ 0, argsbuf);
    if (!args) {
        goto exit;
    }
    if (PyObject_GetBuffer(args[0], &frame_buffer, PyBUF_SIMPLE) != 0) {
        goto exit;
    }
    return_value = _zstd__get_frame_info_impl(module, &frame_buffer);

exit:
    /* Cleanup for frame_buffer */
    if (frame_buffer.obj) {
       PyBuffer_Release(&frame_buffer);
    }

    return return_value;
}

PyDoc_STRVAR(_zstd__set_parameter_types__doc__,
"_set_parameter_types($module, /, c_parameter_type, d_parameter_type)\n"
"--\n"
"\n"
"Internal function, set CParameter/DParameter types for validity check.\n"
"\n"
"  c_parameter_type\n"
"    CParameter IntEnum type object\n"
"  d_parameter_type\n"
"    DParameter IntEnum type object");

#define _ZSTD__SET_PARAMETER_TYPES_METHODDEF    \
    {"_set_parameter_types", _PyCFunction_CAST(_zstd__set_parameter_types), METH_FASTCALL|METH_KEYWORDS, _zstd__set_parameter_types__doc__},

static PyObject *
_zstd__set_parameter_types_impl(PyObject *module, PyObject *c_parameter_type,
                                PyObject *d_parameter_type);

static PyObject *
_zstd__set_parameter_types(PyObject *module, PyObject *const *args, Py_ssize_t nargs, PyObject *kwnames)
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
        .ob_item = { &_Py_ID(c_parameter_type), &_Py_ID(d_parameter_type), },
    };
    #undef NUM_KEYWORDS
    #define KWTUPLE (&_kwtuple.ob_base.ob_base)

    #else  // !Py_BUILD_CORE
    #  define KWTUPLE NULL
    #endif  // !Py_BUILD_CORE

    static const char * const _keywords[] = {"c_parameter_type", "d_parameter_type", NULL};
    static _PyArg_Parser _parser = {
        .keywords = _keywords,
        .fname = "_set_parameter_types",
        .kwtuple = KWTUPLE,
    };
    #undef KWTUPLE
    PyObject *argsbuf[2];
    PyObject *c_parameter_type;
    PyObject *d_parameter_type;

    args = _PyArg_UnpackKeywords(args, nargs, NULL, kwnames, &_parser,
            /*minpos*/ 2, /*maxpos*/ 2, /*minkw*/ 0, /*varpos*/ 0, argsbuf);
    if (!args) {
        goto exit;
    }
    if (!PyObject_TypeCheck(args[0], &PyType_Type)) {
        _PyArg_BadArgument("_set_parameter_types", "argument 'c_parameter_type'", (&PyType_Type)->tp_name, args[0]);
        goto exit;
    }
    c_parameter_type = args[0];
    if (!PyObject_TypeCheck(args[1], &PyType_Type)) {
        _PyArg_BadArgument("_set_parameter_types", "argument 'd_parameter_type'", (&PyType_Type)->tp_name, args[1]);
        goto exit;
    }
    d_parameter_type = args[1];
    return_value = _zstd__set_parameter_types_impl(module, c_parameter_type, d_parameter_type);

exit:
    return return_value;
}

PyDoc_STRVAR(_zstd_compress_stream__doc__,
"compress_stream($module, /, input_stream, output_stream=None,\n"
"                level=None, options=None, zstd_dict=None,\n"
"                pledged_input_size=None, read_size=-1, write_size=-1,\n"
"                callback=None)\n"
"--\n"
"\n"
"Compresses input_stream and writes the compressed data to output_stream.\n"
"\n"
"  input_stream\n"
"    Input stream that has a .readinto(b) method.\n"
"  output_stream\n"
"    Output stream that has a .write(b) method. If using a callback function, this\n"
"    parameter can be None.\n"
"  level\n"
"    The compression level to use, defaults to ZSTD_CLEVEL_DEFAULT.\n"
"  options\n"
"    A dict object that contains advanced compression parameters.\n"
"  zstd_dict\n"
"    A ZstdDict object, a pre-trained zstd dictionary.\n"
"  pledged_input_size\n"
"    If set this parameter to the size of input data, the size will be written into the\n"
"    frame header. If the actual input data doesn\'t match it, a ZstdError will be raised.\n"
"  read_size\n"
"    Input buffer size, in bytes.\n"
"  write_size\n"
"    Output buffer size, in bytes.\n"
"  callback\n"
"    A callback function that accepts four parameters:\n"
"    (total_input, total_output, read_data, write_data)\n"
"    The first two are int objects, the last two are readonly memoryview objects.\n"
"\n"
"This function does not close either stream. If input stream is b\'\', nothing\n"
"will be written to output stream.\n"
"\n"
"Returns a tuple of int objects, (total_input, total_output).");

#define _ZSTD_COMPRESS_STREAM_METHODDEF    \
    {"compress_stream", _PyCFunction_CAST(_zstd_compress_stream), METH_FASTCALL|METH_KEYWORDS, _zstd_compress_stream__doc__},

static PyObject *
_zstd_compress_stream_impl(PyObject *module, PyObject *input_stream,
                           PyObject *output_stream, PyObject *level,
                           PyObject *options, PyObject *zstd_dict,
                           PyObject *pledged_input_size,
                           Py_ssize_t read_size, Py_ssize_t write_size,
                           PyObject *callback);

static PyObject *
_zstd_compress_stream(PyObject *module, PyObject *const *args, Py_ssize_t nargs, PyObject *kwnames)
{
    PyObject *return_value = NULL;
    #if defined(Py_BUILD_CORE) && !defined(Py_BUILD_CORE_MODULE)

    #define NUM_KEYWORDS 9
    static struct {
        PyGC_Head _this_is_not_used;
        PyObject_VAR_HEAD
        PyObject *ob_item[NUM_KEYWORDS];
    } _kwtuple = {
        .ob_base = PyVarObject_HEAD_INIT(&PyTuple_Type, NUM_KEYWORDS)
        .ob_item = { &_Py_ID(input_stream), &_Py_ID(output_stream), &_Py_ID(level), &_Py_ID(options), &_Py_ID(zstd_dict), &_Py_ID(pledged_input_size), &_Py_ID(read_size), &_Py_ID(write_size), &_Py_ID(callback), },
    };
    #undef NUM_KEYWORDS
    #define KWTUPLE (&_kwtuple.ob_base.ob_base)

    #else  // !Py_BUILD_CORE
    #  define KWTUPLE NULL
    #endif  // !Py_BUILD_CORE

    static const char * const _keywords[] = {"input_stream", "output_stream", "level", "options", "zstd_dict", "pledged_input_size", "read_size", "write_size", "callback", NULL};
    static _PyArg_Parser _parser = {
        .keywords = _keywords,
        .fname = "compress_stream",
        .kwtuple = KWTUPLE,
    };
    #undef KWTUPLE
    PyObject *argsbuf[9];
    Py_ssize_t noptargs = nargs + (kwnames ? PyTuple_GET_SIZE(kwnames) : 0) - 1;
    PyObject *input_stream;
    PyObject *output_stream = Py_None;
    PyObject *level = Py_None;
    PyObject *options = Py_None;
    PyObject *zstd_dict = Py_None;
    PyObject *pledged_input_size = Py_None;
    Py_ssize_t read_size = -1;
    Py_ssize_t write_size = -1;
    PyObject *callback = Py_None;

    args = _PyArg_UnpackKeywords(args, nargs, NULL, kwnames, &_parser,
            /*minpos*/ 1, /*maxpos*/ 9, /*minkw*/ 0, /*varpos*/ 0, argsbuf);
    if (!args) {
        goto exit;
    }
    input_stream = args[0];
    if (!noptargs) {
        goto skip_optional_pos;
    }
    if (args[1]) {
        output_stream = args[1];
        if (!--noptargs) {
            goto skip_optional_pos;
        }
    }
    if (args[2]) {
        level = args[2];
        if (!--noptargs) {
            goto skip_optional_pos;
        }
    }
    if (args[3]) {
        options = args[3];
        if (!--noptargs) {
            goto skip_optional_pos;
        }
    }
    if (args[4]) {
        zstd_dict = args[4];
        if (!--noptargs) {
            goto skip_optional_pos;
        }
    }
    if (args[5]) {
        pledged_input_size = args[5];
        if (!--noptargs) {
            goto skip_optional_pos;
        }
    }
    if (args[6]) {
        {
            Py_ssize_t ival = -1;
            PyObject *iobj = _PyNumber_Index(args[6]);
            if (iobj != NULL) {
                ival = PyLong_AsSsize_t(iobj);
                Py_DECREF(iobj);
            }
            if (ival == -1 && PyErr_Occurred()) {
                goto exit;
            }
            read_size = ival;
        }
        if (!--noptargs) {
            goto skip_optional_pos;
        }
    }
    if (args[7]) {
        {
            Py_ssize_t ival = -1;
            PyObject *iobj = _PyNumber_Index(args[7]);
            if (iobj != NULL) {
                ival = PyLong_AsSsize_t(iobj);
                Py_DECREF(iobj);
            }
            if (ival == -1 && PyErr_Occurred()) {
                goto exit;
            }
            write_size = ival;
        }
        if (!--noptargs) {
            goto skip_optional_pos;
        }
    }
    callback = args[8];
skip_optional_pos:
    return_value = _zstd_compress_stream_impl(module, input_stream, output_stream, level, options, zstd_dict, pledged_input_size, read_size, write_size, callback);

exit:
    return return_value;
}

PyDoc_STRVAR(_zstd_decompress_stream__doc__,
"decompress_stream($module, /, input_stream, output_stream=None,\n"
"                  options=None, zstd_dict=None, read_size=-1,\n"
"                  write_size=-1, callback=None)\n"
"--\n"
"\n"
"Decompresses input_stream and writes the decompressed data to output_stream.asm\n"
"\n"
"  input_stream\n"
"    Input stream that has a .readinto(b) method.\n"
"  output_stream\n"
"    Output stream that has a .write(b) method. If using a callback function, this\n"
"    parameter can be None.\n"
"  options\n"
"    A dict object that contains advanced decompression parameters.\n"
"  zstd_dict\n"
"    A ZstdDict object, a pre-trained zstd dictionary.\n"
"  read_size\n"
"    Input buffer size, in bytes.\n"
"  write_size\n"
"    Output buffer size, in bytes.\n"
"  callback\n"
"    A callback function that accepts four parameters:\n"
"    (total_input, total_output, read_data, write_data)\n"
"    The first two are int objects, the last two are readonly memoryview objects.\n"
"\n"
"This function does not close either stream. Supports multiple concatenated frames.\n"
"Returns a tuple of int objects, (total_input, total_output).");

#define _ZSTD_DECOMPRESS_STREAM_METHODDEF    \
    {"decompress_stream", _PyCFunction_CAST(_zstd_decompress_stream), METH_FASTCALL|METH_KEYWORDS, _zstd_decompress_stream__doc__},

static PyObject *
_zstd_decompress_stream_impl(PyObject *module, PyObject *input_stream,
                             PyObject *output_stream, PyObject *options,
                             PyObject *zstd_dict, Py_ssize_t read_size,
                             Py_ssize_t write_size, PyObject *callback);

static PyObject *
_zstd_decompress_stream(PyObject *module, PyObject *const *args, Py_ssize_t nargs, PyObject *kwnames)
{
    PyObject *return_value = NULL;
    #if defined(Py_BUILD_CORE) && !defined(Py_BUILD_CORE_MODULE)

    #define NUM_KEYWORDS 7
    static struct {
        PyGC_Head _this_is_not_used;
        PyObject_VAR_HEAD
        PyObject *ob_item[NUM_KEYWORDS];
    } _kwtuple = {
        .ob_base = PyVarObject_HEAD_INIT(&PyTuple_Type, NUM_KEYWORDS)
        .ob_item = { &_Py_ID(input_stream), &_Py_ID(output_stream), &_Py_ID(options), &_Py_ID(zstd_dict), &_Py_ID(read_size), &_Py_ID(write_size), &_Py_ID(callback), },
    };
    #undef NUM_KEYWORDS
    #define KWTUPLE (&_kwtuple.ob_base.ob_base)

    #else  // !Py_BUILD_CORE
    #  define KWTUPLE NULL
    #endif  // !Py_BUILD_CORE

    static const char * const _keywords[] = {"input_stream", "output_stream", "options", "zstd_dict", "read_size", "write_size", "callback", NULL};
    static _PyArg_Parser _parser = {
        .keywords = _keywords,
        .fname = "decompress_stream",
        .kwtuple = KWTUPLE,
    };
    #undef KWTUPLE
    PyObject *argsbuf[7];
    Py_ssize_t noptargs = nargs + (kwnames ? PyTuple_GET_SIZE(kwnames) : 0) - 1;
    PyObject *input_stream;
    PyObject *output_stream = Py_None;
    PyObject *options = Py_None;
    PyObject *zstd_dict = Py_None;
    Py_ssize_t read_size = -1;
    Py_ssize_t write_size = -1;
    PyObject *callback = Py_None;

    args = _PyArg_UnpackKeywords(args, nargs, NULL, kwnames, &_parser,
            /*minpos*/ 1, /*maxpos*/ 7, /*minkw*/ 0, /*varpos*/ 0, argsbuf);
    if (!args) {
        goto exit;
    }
    input_stream = args[0];
    if (!noptargs) {
        goto skip_optional_pos;
    }
    if (args[1]) {
        output_stream = args[1];
        if (!--noptargs) {
            goto skip_optional_pos;
        }
    }
    if (args[2]) {
        options = args[2];
        if (!--noptargs) {
            goto skip_optional_pos;
        }
    }
    if (args[3]) {
        zstd_dict = args[3];
        if (!--noptargs) {
            goto skip_optional_pos;
        }
    }
    if (args[4]) {
        {
            Py_ssize_t ival = -1;
            PyObject *iobj = _PyNumber_Index(args[4]);
            if (iobj != NULL) {
                ival = PyLong_AsSsize_t(iobj);
                Py_DECREF(iobj);
            }
            if (ival == -1 && PyErr_Occurred()) {
                goto exit;
            }
            read_size = ival;
        }
        if (!--noptargs) {
            goto skip_optional_pos;
        }
    }
    if (args[5]) {
        {
            Py_ssize_t ival = -1;
            PyObject *iobj = _PyNumber_Index(args[5]);
            if (iobj != NULL) {
                ival = PyLong_AsSsize_t(iobj);
                Py_DECREF(iobj);
            }
            if (ival == -1 && PyErr_Occurred()) {
                goto exit;
            }
            write_size = ival;
        }
        if (!--noptargs) {
            goto skip_optional_pos;
        }
    }
    callback = args[6];
skip_optional_pos:
    return_value = _zstd_decompress_stream_impl(module, input_stream, output_stream, options, zstd_dict, read_size, write_size, callback);

exit:
    return return_value;
}

PyDoc_STRVAR(_zstd_compress__doc__,
"compress($module, /, data, level=<unrepresentable>,\n"
"         options=<unrepresentable>, zstd_dict=<unrepresentable>)\n"
"--\n"
"\n"
"Compress data, return a bytes object of zstd compressed data.\n"
"\n"
"  data\n"
"    A bytes-like object, data to be compressed.\n"
"  level\n"
"    The compression level to use, defaults to ZSTD_CLEVEL_DEFAULT.\n"
"  options\n"
"    A dict object that contains advanced compression parameters.\n"
"  zstd_dict\n"
"    A ZstdDict object, a pre-trained zstd dictionary.");

#define _ZSTD_COMPRESS_METHODDEF    \
    {"compress", _PyCFunction_CAST(_zstd_compress), METH_FASTCALL|METH_KEYWORDS, _zstd_compress__doc__},

static PyObject *
_zstd_compress_impl(PyObject *module, Py_buffer *data, PyObject *level,
                    PyObject *options, PyObject *zstd_dict);

static PyObject *
_zstd_compress(PyObject *module, PyObject *const *args, Py_ssize_t nargs, PyObject *kwnames)
{
    PyObject *return_value = NULL;
    #if defined(Py_BUILD_CORE) && !defined(Py_BUILD_CORE_MODULE)

    #define NUM_KEYWORDS 4
    static struct {
        PyGC_Head _this_is_not_used;
        PyObject_VAR_HEAD
        PyObject *ob_item[NUM_KEYWORDS];
    } _kwtuple = {
        .ob_base = PyVarObject_HEAD_INIT(&PyTuple_Type, NUM_KEYWORDS)
        .ob_item = { &_Py_ID(data), &_Py_ID(level), &_Py_ID(options), &_Py_ID(zstd_dict), },
    };
    #undef NUM_KEYWORDS
    #define KWTUPLE (&_kwtuple.ob_base.ob_base)

    #else  // !Py_BUILD_CORE
    #  define KWTUPLE NULL
    #endif  // !Py_BUILD_CORE

    static const char * const _keywords[] = {"data", "level", "options", "zstd_dict", NULL};
    static _PyArg_Parser _parser = {
        .keywords = _keywords,
        .fname = "compress",
        .kwtuple = KWTUPLE,
    };
    #undef KWTUPLE
    PyObject *argsbuf[4];
    Py_ssize_t noptargs = nargs + (kwnames ? PyTuple_GET_SIZE(kwnames) : 0) - 1;
    Py_buffer data = {NULL, NULL};
    PyObject *level = NULL;
    PyObject *options = NULL;
    PyObject *zstd_dict = NULL;

    args = _PyArg_UnpackKeywords(args, nargs, NULL, kwnames, &_parser,
            /*minpos*/ 1, /*maxpos*/ 4, /*minkw*/ 0, /*varpos*/ 0, argsbuf);
    if (!args) {
        goto exit;
    }
    if (PyObject_GetBuffer(args[0], &data, PyBUF_SIMPLE) != 0) {
        goto exit;
    }
    if (!noptargs) {
        goto skip_optional_pos;
    }
    if (args[1]) {
        level = args[1];
        if (!--noptargs) {
            goto skip_optional_pos;
        }
    }
    if (args[2]) {
        options = args[2];
        if (!--noptargs) {
            goto skip_optional_pos;
        }
    }
    zstd_dict = args[3];
skip_optional_pos:
    return_value = _zstd_compress_impl(module, &data, level, options, zstd_dict);

exit:
    /* Cleanup for data */
    if (data.obj) {
       PyBuffer_Release(&data);
    }

    return return_value;
}

PyDoc_STRVAR(_zstd_decompress__doc__,
"decompress($module, /, data, zstd_dict=<unrepresentable>,\n"
"           options=<unrepresentable>)\n"
"--\n"
"\n"
"Decompress zstd data, return a bytes object.\n"
"\n"
"  data\n"
"    A bytes-like object, zstd data to be decompressed.\n"
"  zstd_dict\n"
"    A ZstdDict object, a pre-trained zstd dictionary.\n"
"  options\n"
"    A dict object that contains advanced decompression parameters.\n"
"\n"
"Supports multiple concatenated frames.");

#define _ZSTD_DECOMPRESS_METHODDEF    \
    {"decompress", _PyCFunction_CAST(_zstd_decompress), METH_FASTCALL|METH_KEYWORDS, _zstd_decompress__doc__},

static PyObject *
_zstd_decompress_impl(PyObject *module, Py_buffer *data, PyObject *zstd_dict,
                      PyObject *options);

static PyObject *
_zstd_decompress(PyObject *module, PyObject *const *args, Py_ssize_t nargs, PyObject *kwnames)
{
    PyObject *return_value = NULL;
    #if defined(Py_BUILD_CORE) && !defined(Py_BUILD_CORE_MODULE)

    #define NUM_KEYWORDS 3
    static struct {
        PyGC_Head _this_is_not_used;
        PyObject_VAR_HEAD
        PyObject *ob_item[NUM_KEYWORDS];
    } _kwtuple = {
        .ob_base = PyVarObject_HEAD_INIT(&PyTuple_Type, NUM_KEYWORDS)
        .ob_item = { &_Py_ID(data), &_Py_ID(zstd_dict), &_Py_ID(options), },
    };
    #undef NUM_KEYWORDS
    #define KWTUPLE (&_kwtuple.ob_base.ob_base)

    #else  // !Py_BUILD_CORE
    #  define KWTUPLE NULL
    #endif  // !Py_BUILD_CORE

    static const char * const _keywords[] = {"data", "zstd_dict", "options", NULL};
    static _PyArg_Parser _parser = {
        .keywords = _keywords,
        .fname = "decompress",
        .kwtuple = KWTUPLE,
    };
    #undef KWTUPLE
    PyObject *argsbuf[3];
    Py_ssize_t noptargs = nargs + (kwnames ? PyTuple_GET_SIZE(kwnames) : 0) - 1;
    Py_buffer data = {NULL, NULL};
    PyObject *zstd_dict = NULL;
    PyObject *options = NULL;

    args = _PyArg_UnpackKeywords(args, nargs, NULL, kwnames, &_parser,
            /*minpos*/ 1, /*maxpos*/ 3, /*minkw*/ 0, /*varpos*/ 0, argsbuf);
    if (!args) {
        goto exit;
    }
    if (PyObject_GetBuffer(args[0], &data, PyBUF_SIMPLE) != 0) {
        goto exit;
    }
    if (!noptargs) {
        goto skip_optional_pos;
    }
    if (args[1]) {
        zstd_dict = args[1];
        if (!--noptargs) {
            goto skip_optional_pos;
        }
    }
    options = args[2];
skip_optional_pos:
    return_value = _zstd_decompress_impl(module, &data, zstd_dict, options);

exit:
    /* Cleanup for data */
    if (data.obj) {
       PyBuffer_Release(&data);
    }

    return return_value;
}
/*[clinic end generated code: output=abac144eb5913005 input=a9049054013a1b77]*/

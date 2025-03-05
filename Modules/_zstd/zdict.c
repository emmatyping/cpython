/*
Low level interface to Meta's zstd library for use in the `zstd` Python library.

Original implementation by Ma Lin with patches by Rogdham.

Refactored for the CPython standard library by Emma Harper Smith.
*/

/* ZstdDict class definitions */

/*[clinic input]
module _zstd
class _zstd.ZstdDict "PyObject *" "clinic_state()->ZstdDict_type"
[clinic start generated code]*/
/*[clinic end generated code: output=da39a3ee5e6b4b0d input=4512bf4797fd0f46]*/

#ifndef Py_BUILD_CORE_BUILTIN
#  define Py_BUILD_CORE_MODULE 1
#endif

#include "_zstdmodule.h"

/* -----------------
     ZstdDict code
   ----------------- */

/*[clinic input]
@classmethod
_zstd.ZstdDict.__new__

    args: object = NULL
    *
    kwargs: object = NULL

[clinic start generated code]*/

static PyObject *
_zstd_ZstdDict_impl(PyTypeObject *type, PyObject *args, PyObject *kwargs)
/*[clinic end generated code: output=bac595bcbfb6a075 input=f7c8d61820baca39]*/
{
    ZstdDict *self;
    self = (ZstdDict*)type->tp_alloc(type, 0);
    if (self == NULL) {
        goto error;
    }

    /* Keep this first. Set module state to self. */
    SET_STATE_TO_OBJ(type, self);

    assert(self->dict_content == NULL);
    assert(self->dict_id == 0);
    assert(self->d_dict == NULL);
    assert(self->inited == 0);

    /* ZSTD_CDict dict */
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
    /* Free ZSTD_CDict instances */
    Py_XDECREF(self->c_dicts);

    /* Free ZSTD_DDict instance */
    ZSTD_freeDDict(self->d_dict);

    /* Release dict_content after Free ZSTD_CDict/ZSTD_DDict instances */
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

    self: self(type="ZstdDict *")
    dict_content: object
        A bytes-like object, dictionary's content.
    is_raw: bool = False
        This parameter is for advanced user. True means dict_content
        argument is a "raw content" dictionary, free of any format
        restriction. False means dict_content argument is an ordinary
        zstd dictionary, was created by zstd functions, follow a
        specified format.

Initialize a ZstdDict object.
[clinic start generated code]*/

static int
_zstd_ZstdDict___init___impl(ZstdDict *self, PyObject *dict_content,
                             int is_raw)
/*[clinic end generated code: output=c5f5a0d8377d037c input=1b4fba2f7acac768]*/
{
    /* Only called once */
    if (self->inited) {
        PyErr_SetString(PyExc_RuntimeError, init_twice_msg);
        return -1;
    }
    self->inited = 1;

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


/*[clinic input]
_zstd.ZstdDict.__reduce__

Intentionally not supporting pickle.
[clinic start generated code]*/

static PyObject *
_zstd_ZstdDict___reduce___impl(PyObject *self)
/*[clinic end generated code: output=e20d2153a89fd03c input=3b3bf01bc131f928]*/
{
    /* return Py_BuildValue("O(On)", Py_TYPE(self),
                            self->dict_content,
                            self->dict_id == 0);
       v0.15.7 added .as_* attributes, pickle will cause more confusion. */
    PyErr_SetString(PyExc_TypeError,
                    "ZstdDict object intentionally doesn't support pickle. If you need "
                    "to save zstd dictionary to disk, please save .dict_content "
                    "attribute, it's a bytes object. So that the zstd dictionary "
                    "can be used with other programs.");
    return NULL;
}

#define clinic_state() ((_zstd_state*)PyType_GetModuleState(type))
#include "clinic/zdict.c.h"
#undef clinic_state

static PyMethodDef ZstdDict_methods[] = {
    _ZSTD_ZSTDDICT___REDUCE___METHODDEF
    {0}
};

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

/*[clinic input]
@critical_section
@getter
_zstd.ZstdDict.as_digested_dict

Load as a digested dictionary to compressor.

Pass this attribute as zstd_dict argument: compress(dat, zstd_dict=zd.as_digested_dict)
1. Some advanced compression parameters of compressor may be overridden
   by parameters of digested dictionary.
2. ZstdDict has a digested dictionaries cache for each compression level.
   It's faster when loading again a digested dictionary with the same
   compression level.
3. No need to use this for decompression.
[clinic start generated code]*/

static PyObject *
_zstd_ZstdDict_as_digested_dict_get_impl(PyObject *self)
/*[clinic end generated code: output=eb7396856b57498f input=585448c79f31f74a]*/
{
    return Py_BuildValue("Oi", self, DICT_TYPE_DIGESTED);
}

/*[clinic input]
@critical_section
@getter
_zstd.ZstdDict.as_undigested_dict

Load as an undigested dictionary to compressor.

Pass this attribute as zstd_dict argument: compress(dat, zstd_dict=zd.as_undigested_dict)
1. The advanced compression parameters of compressor will not be overridden.
2. Loading an undigested dictionary is costly. If load an undigested dictionary
   multiple times, consider reusing a compressor object.
3. No need to use this for decompression.
[clinic start generated code]*/

static PyObject *
_zstd_ZstdDict_as_undigested_dict_get_impl(PyObject *self)
/*[clinic end generated code: output=9ae901a7b3ad1aad input=022b0829ffb1c220]*/
{
    return Py_BuildValue("Oi", self, DICT_TYPE_UNDIGESTED);
}

/*[clinic input]
@critical_section
@getter
_zstd.ZstdDict.as_prefix

Load as a prefix to compressor/decompressor.

Pass this attribute as zstd_dict argument: compress(dat, zstd_dict=zd.as_prefix)
1. Prefix is compatible with long distance matching, while dictionary is not.
2. It only works for the first frame, then the compressor/decompressor will
   return to no prefix state.
3. When decompressing, must use the same prefix as when compressing."
[clinic start generated code]*/

static PyObject *
_zstd_ZstdDict_as_prefix_get_impl(PyObject *self)
/*[clinic end generated code: output=d460d8be7450cae8 input=09fb82a6a5407e87]*/
{
    return Py_BuildValue("Oi", self, DICT_TYPE_PREFIX);
}

static PyGetSetDef ZstdDict_getset[] = {
    _ZSTD_ZSTDDICT_AS_DIGESTED_DICT_GETSETDEF

    _ZSTD_ZSTDDICT_AS_UNDIGESTED_DICT_GETSETDEF

    _ZSTD_ZSTDDICT_AS_PREFIX_GETSETDEF

    {0}
};

static Py_ssize_t
ZstdDict_length(ZstdDict *self)
{
    assert(PyBytes_Check(self->dict_content));
    return Py_SIZE(self->dict_content);
}

static PyType_Slot zstddict_slots[] = {
    {Py_tp_methods, ZstdDict_methods},
    {Py_tp_members, ZstdDict_members},
    {Py_tp_getset, ZstdDict_getset},
    {Py_tp_new, _zstd_ZstdDict},
    {Py_tp_dealloc, ZstdDict_dealloc},
    {Py_tp_init, _zstd_ZstdDict___init__},
    {Py_tp_str, ZstdDict_str},
    {Py_tp_doc, (char*)_zstd_ZstdDict___init____doc__},
    {Py_sq_length, ZstdDict_length},
    {0}
};

PyType_Spec zstddict_type_spec = {
    .name = "_zstd.ZstdDict",
    .basicsize = sizeof(ZstdDict),
    .flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .slots = zstddict_slots,
};

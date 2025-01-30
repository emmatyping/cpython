/*
Low level interface to Meta's zstd library for use in the `zstd` Python library.

Original implementation by Ma Lin with patches by Rogdham.

Refactored for the CPython standard library by Emma Harper Smith.
*/

/* Declarations shared between different parts of the _zstd module*/

#include "Python.h"

#include "zstd.h"
#include "zdict.h"

// TODO(emmatyping): should we put a different policy on minimum supported libzstd?
#if ZSTD_VERSION_NUMBER < 10400
    #error "_zstd module requires zstd v1.4.0+"
#endif

#include "pycore_blocks_output_buffer.h"


/* Forward declaration of module state */
typedef struct _zstd_state _zstd_state;

/* Get module state from a class type, and set it to supported object.
    Used in Py_tp_new or Py_tp_init. */
#define SET_STATE_TO_OBJ(type, obj) \
    do {                                                                \
        (obj)->module_state = (_zstd_state*)PyType_GetModuleState(type);\
        if ((obj)->module_state == NULL) {                              \
            goto error;                                                 \
        }                                                               \
    } while (0)
/* Get module state from module object */
#define STATE_FROM_MODULE(module) \
    _zstd_state* const _module_state = (_zstd_state*)PyModule_GetState(module); \
    assert(_module_state != NULL);
/* Get module state from supported object */
#define STATE_FROM_OBJ(obj) \
    _zstd_state* const _module_state = (obj)->module_state; \
    assert(_module_state != NULL);
/* Place as module state. Only as r-value. */
#define MODULE_STATE (1 ? _module_state : NULL)
/* Access a member of module state. Can be l-value or r-value. */
#define MS_MEMBER(member) (_module_state->member)
/* Get state for clinic generated wrappers */


extern PyType_Spec zstddict_type_spec;
/*
extern PyType_Spec zstdcompressor_type_spec;
extern PyType_Spec ZstdDecompressor_type_spec;
extern PyType_Spec EndlessZstdDecompressor_type_spec;
extern PyType_Spec ZstdFileReader_type_spec;
extern PyType_Spec ZstdFileWriter_type_spec;
*/

struct _zstd_state {
    PyObject *empty_bytes;
    PyObject *empty_readonly_memoryview;
    PyObject *str_read;
    PyObject *str_readinto;
    PyObject *str_write;
    PyObject *str_flush;

    PyTypeObject *ZstdDict_type;
    /*PyTypeObject *ZstdCompressor_type;
    PyTypeObject *RichMemZstdCompressor_type;
    PyTypeObject *ZstdDecompressor_type;
    PyTypeObject *EndlessZstdDecompressor_type;
    PyTypeObject *ZstdFileReader_type;
    PyTypeObject *ZstdFileWriter_type;*/
    PyObject *ZstdError;

    //PyTypeObject *CParameter_type;
    //PyTypeObject *DParameter_type;
};

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
extern void
set_zstd_error(const _zstd_state* const state,
               const error_type type, const size_t zstd_ret);

static const char init_twice_msg[] = "__init__ method is called twice.";

PyDoc_STRVAR(reduce_cannot_pickle_doc,
"Intentionally not supporting pickle.");

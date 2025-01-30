# _zstd module

This module is based on the `pyzstd` project.

## Overview of changes from pyzstd in this branch

1. Compatibility shims for older Python versions are removed.
2. Multi-phase init is enabled by default
3. PyDoc_StrVar is replaced with argument clinic where possible.
4. Removed forced inlining/no inlining
5. replaced the inclusion of other .c files in `_zstd.c` with the creation of multiple object files to be linked in to `_zstd.c`. This should make compilation faster and the code easier to understand.
6. stream.c was removed to move the module level functions into _zstdmodule.c


TODOs:
1. ~~figure out what to do about inlining~~
2. figure out minimum zstd version to support
3. rewrite invoke_method_*_arg
4. Pickle support?
5. ~~Figure out what to do about mremap for the output buffer~~ I think it will just be conditional on `HAVE_MREMAP`
6. for compress_stream, how to handle the defaults/why the size is under an assert
7. Should we provide a `_zstd.compress(bytes)`?
8. ^ or generally provide buffer protocol support?
9. Windows build system support (ref https://devguide.python.org/developer-workflow/extension-modules/#updating-msvc-project-files)

# _zstd module

This module is based on the `pyzstd` project.

## Overview of changes from pyzstd in this branch

1. Compatibility shims for older Python versions are removed.
2. Multi-phase init is enabled by default
3. PyDoc_StrVar is replaced with argument clinic where possible.
4. Removed forced inlining/no inlining
5. Simplified API by removing ZstdEndless(De)Compressor
6. replaced the inclusion of other .c files in `_zstd.c` with the creation of multiple object files to be linked in to `_zstd.c`. This should make compilation faster.


TODOs:
1. ~~figure out what to do about inlining~~
2. figure out minimum zstd version to support
3. rewrite invoke_method_*_arg
4. Pickle support?
5. Figure out what to do about mremap for the output buffer
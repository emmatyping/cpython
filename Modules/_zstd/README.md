# _zstd module

This module is based on the `pyzstd` project.

## Overview of changes from pyzstd in this branch

1. Compatibility shims for older Python versions are removed.
2. Multi-phase init is enabled by default
3. PyDoc_StrVar/manual argument parsing is replaced with argument clinic where possible.
4. Removed forced inlining/no inlining
5. replaced the inclusion of other .c files in `_zstd.c` with the creation of multiple object files to be linked in to `_zstd.c`. This should make compilation faster and the code easier to understand.
6. "RichMem" is used by default, which boils down to calculating the output buffer size up-front.
7. The ZSTD_c_targetCBlockSize parameter is omitted as the minimum zstd version is not yet v1.5.6
8. This implementation uses the standard library output buffer utility
9. level_or_option replaced with independent level and option kwargs


TODOs:
1. Implement streams. for compress_stream, how to handle the defaults/why the size is under an assert
2. Windows build system support (ref https://devguide.python.org/developer-workflow/extension-modules/#updating-msvc-project-files)
3. Go over class __init__/class docs to see if they should be refactored
4. Compile guard for OUTPUT_BUFFER_MAX_BLOCK_SIZE ?
5. Tests
6. Fuzzing
7. Docs
8. PEP

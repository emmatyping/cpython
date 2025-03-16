# _zstd module

This module is based on the `pyzstd` project.

## Overview of changes from pyzstd in this branch

1. Compatibility shims for older Python versions are removed.
2. Multi-phase init is enabled by default
3. PyDoc_StrVar/manual argument parsing is replaced with argument clinic where possible.
4. Removed forced inlining/no inlining
5. replaced the inclusion of other .c files in `_zstd.c` with the creation of multiple object files to be linked in to `_zstd.c`. This should make compilation faster and the code easier to understand.
6. "RichMem" is used by default more or less, which boils down to calculating the output buffer size up-front. We don't use memremap buffers, but this could be contributed separately
7. The ZSTD_c_targetCBlockSize parameter is omitted as the minimum zstd version is not yet v1.5.6
8. This implementation uses the standard library output buffer utility
9. level_or_option replaced with independent level and options kwargs
10. ZstdFileReader and ZstdFileWriter were removed as ZstdFile is implemented in Python


TODOs:
1. Windows build system support (ref https://devguide.python.org/developer-workflow/extension-modules/#updating-msvc-project-files)
2. More tests!
3. Fuzzing (upstream libfuzzer integration and use it)
4. PEP

After-PEP posting:
1. Go over class __init__/class docs to see if they should be refactored
2. Ask about compile guard for OUTPUT_BUFFER_MAX_BLOCK_SIZE ?
3. TODOs in code base
4. Docs (maybe before PEP)

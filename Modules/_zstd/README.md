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
11. `_set_pledged_input_size`, `_reset_session`, and `(de)compress_stream` were all removed
12. `ZstdDict.as_prefix` is removed, to keep the initial API simpler.

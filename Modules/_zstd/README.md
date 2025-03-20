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
1. ~~Add pyzstd license~~
2. ~~Remove seekable_zstdfile~~
3. ~~Make and use `compression` namespace a la hashlib (and re-export other compression formats)~~
4. ~~Set thread defaults~~
5. ~~More tests!~~
6. ~~Remove `_set_pledged_input_size`~~
7. ~~Remove `_reset_session`~~
8. ~~Remove `(de)compress_stream`~~
9. Add integrations with other modules (i.e. zipfile, tarfile)
10. Go over class __init__/class docs to see if they should be refactored
11. Documentation
12. Windows build system support (ref https://devguide.python.org/developer-workflow/extension-modules/#updating-msvc-project-files)
13. Draft PEP

After-PEP posting:
1. Ask about compile guard for OUTPUT_BUFFER_MAX_BLOCK_SIZE ?
2. TODOs in code base
3. Fuzzing (upstream libfuzzer integration and use it)

PEP open questions:
1. When to deprecate `import lzma` etc?
2. Should we remove `EndlessZstdDecompressor`? (leaning towards no)
3. Should arguments (i.e. options) be made more Pythonic? If so, how?

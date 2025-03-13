# _zstd module

This module is based on the `pyzstd` project.

## Overview of changes from pyzstd in this branch

1. Compatibility shims for older Python versions are removed.
2. Multi-phase init is enabled by default
3. PyDoc_StrVar is replaced with argument clinic where possible.
4. Removed forced inlining/no inlining
5. replaced the inclusion of other .c files in `_zstd.c` with the creation of multiple object files to be linked in to `_zstd.c`. This should make compilation faster and the code easier to understand.
6. "RichMem" is used by default, which boils down to calculating the output buffer size up-front.
7. The ZSTD_c_targetCBlockSize parameter is omitted as the minimum zstd version is not yet v1.5.6


TODOs:
1. ~~figure out what to do about inlining~~
2. ~~figure out minimum zstd version to support~~ v1.4.5
3. ~~rewrite invoke_method_*_arg~~ Not needed with Python-implemented file API
4. ~~Pickle support?~~ ZstdDict gives an informative error. Other classes shouldn't need support.
5. ~~Figure out what to do about mremap for the output buffer~~ ~~I think it will just be conditional on `HAVE_MREMAP`, but added after the initial code.~~ Actually, we can just use "richmem"
6. for compress_stream, how to handle the defaults/why the size is under an assert
7. ~~Should we provide a `zstd.compress(bytes)`?~~ yes, it is done
8. ~~Should we support buffer protocol for ZstdDict training and finalization?~~ No
9. Windows build system support (ref https://devguide.python.org/developer-workflow/extension-modules/#updating-msvc-project-files)
10. ~~Is there a better way to handle `__new__` with clinic?~~ New is fine, `__init__` is a bit clunky
11. ~~What to do about parameter values?~~
12. ~~Should `EndlessZstdDecompressor` be renamed?~~ Defer on this until PEP, default to keeping the same
13. Go over class __init__/class docs to see if they should be refactored
14. ~~Go through any compile time zstd verison checks and see if they need to moved into runtime checks~~
15. ~~TODOs in the code base~~
16. ~~Should ZstdCompressor.compress() take an initial size argument?~~ Keep the same API
17. Compile guard for OUTPUT_BUFFER_MAX_BLOCK_SIZE ?
18. Tests
19. Fuzzing
20. Docs
21. PEP

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


TODOs:
1. ~~Add pyzstd license~~
2. ~~Remove seekable_zstdfile~~
3. ~~Make and use `compression` namespace a la hashlib (and re-export other compression formats)~~
4. ~~Set thread defaults~~
5. ~~More tests!~~
6. ~~Remove `_set_pledged_input_size`~~
7. ~~Remove `_reset_session`~~
8. ~~Remove `(de)compress_stream`~~
9. ~~Add integrations with other modules (i.e. zipfile, tarfile, shutil)~~
10. ~~Documentation~~
11. ~~Go over zstdfile and make sure it supports multiple frames concatenated together (might need to use `EndlessZstdDecompressor`)~~ handled by compression._common.streams.BaseStream, see the `test_read_multistream` test.
12. ~~Ensure docs on zstdfile matching BufferedIOBase is correct (i.e. with/iteration/list of methods)~~ with/iter exercised in `test_iterator`. Verified it matches the shape of BufferedIOBase.
13. ~~Draft PEP~~

After-PEP posting:
1. Ask about compile guard for OUTPUT_BUFFER_MAX_BLOCK_SIZE ?
2. ~~TODOs in code base~~
3. ~~Go over class `__init__`/class docs to see if they should be refactored~~
4. Windows build system support (ref https://devguide.python.org/developer-workflow/extension-modules/#updating-msvc-project-files). Still not sure how to add zstd properly to cpython-source-deps
5. Add test for method 20 reading to test_zipfile
6. Defaults and more tests for threading
7. Add docs for CParameter/DParameter
8. Add deprecations for existing compression libraries
9. Fuzzing (upstream libfuzzer integration and use it)

PEP/Review open questions:
1. When to deprecate `import lzma` etc?
2. Should we remove `EndlessZstdDecompressor`? (leaning towards yes)
3. Should arguments (i.e. options) be made more Pythonic? If so, how? (should `level` be renamed to `compresslevel`?)
4. Should the compressor only be able to handle a single frame? The underlying APIs allow compressing more than one frame.

"""Python bindings to Zstandard (zstd) compression library, the API style is
similar to Python's bz2/lzma/zlib modules.

Command line interface of this module: python -m pyzstd --help
"""

__all__ = (  # From this file
    "compressionLevel_values",
    "get_frame_info",
    "finalize_dict",
    "train_dict",
    "zstd_support_multithread",
    # From _zstd
    "ZstdCompressor",
    "ZstdDecompressor",
    "ZstdDict",
    "ZstdError",
    "compress",
    "decompress",
    "get_frame_size",
    "zstd_version",
    "zstd_version_info",
    "get_param_bounds"
    # From zstd.zstdfile
    "open",
    "ZstdFile",
)

from collections import namedtuple
from enum import IntEnum
from functools import lru_cache

from _zstd import *
import _zstd
from .zstdfile import ZstdFile, open


_train_dict = _zstd._train_dict
_finalize_dict = _zstd._finalize_dict


# compressionLevel_values
_nt_values = namedtuple("values", ["default", "min", "max"])
compressionLevel_values = _nt_values(*_zstd._compressionLevel_values)


_nt_frame_info = namedtuple("frame_info", ["decompressed_size", "dictionary_id"])


def get_frame_info(frame_buffer):
    """Get zstd frame information from a frame header.

    Parameter
    frame_buffer: A bytes-like object. It should starts from the beginning of
                  a frame, and needs to include at least the frame header (6 to
                  18 bytes).

    Return a two-items namedtuple: (decompressed_size, dictionary_id)

    If decompressed_size is None, decompressed size is unknown.

    dictionary_id is a 32-bit unsigned integer value. 0 means dictionary ID was
    not recorded in the frame header, the frame may or may not need a dictionary
    to be decoded, and the ID of such a dictionary is not specified.

    It's possible to append more items to the namedtuple in the future."""

    ret_tuple = _zstd._get_frame_info(frame_buffer)
    return _nt_frame_info(*ret_tuple)


def _nbytes(dat):
    if isinstance(dat, (bytes, bytearray)):
        return len(dat)
    with memoryview(dat) as mv:
        return mv.nbytes


def train_dict(samples, dict_size):
    """Train a zstd dictionary, return a ZstdDict object.

    Parameters
    samples:   An iterable of samples, a sample is a bytes-like object
               represents a file.
    dict_size: The dictionary's maximum size, in bytes.
    """
    # Check argument's type
    if not isinstance(dict_size, int):
        raise TypeError('dict_size argument should be an int object.')

    # Prepare data
    chunks = []
    chunk_sizes = []
    for chunk in samples:
        chunks.append(chunk)
        chunk_sizes.append(_nbytes(chunk))

    chunks = b''.join(chunks)
    if not chunks:
        raise ValueError("The samples are empty content, can't train dictionary.")

    # samples_bytes: samples be stored concatenated in a single flat buffer.
    # samples_size_list: a list of each sample's size.
    # dict_size: size of the dictionary, in bytes.
    dict_content = _train_dict(chunks, chunk_sizes, dict_size)

    return ZstdDict(dict_content)


def finalize_dict(zstd_dict, samples, dict_size, level):
    """Finalize a zstd dictionary, return a ZstdDict object.

    Given a custom content as a basis for dictionary, and a set of samples,
    finalize dictionary by adding headers and statistics according to the zstd
    dictionary format.

    You may compose an effective dictionary content by hand, which is used as
    basis dictionary, and use some samples to finalize a dictionary. The basis
    dictionary can be a "raw content" dictionary, see is_raw parameter in
    ZstdDict.__init__ method.

    Parameters
    zstd_dict: A ZstdDict object, basis dictionary.
    samples:   An iterable of samples, a sample is a bytes-like object
               represents a file.
    dict_size: The dictionary's maximum size, in bytes.
    level:     The compression level expected to use in production. The
               statistics for each compression level differ, so tuning the
               dictionary for the compression level can help quite a bit.
    """
    if zstd_version_info < (1, 4, 5):
        msg = ("This function only available when the underlying zstd "
               "library's version is greater than or equal to v1.4.5, "
               "the current underlying zstd library's version is v%s.") % zstd_version
        raise NotImplementedError(msg)

    # Check arguments' type
    if not isinstance(zstd_dict, ZstdDict):
        raise TypeError('zstd_dict argument should be a ZstdDict object.')
    if not isinstance(dict_size, int):
        raise TypeError('dict_size argument should be an int object.')
    if not isinstance(level, int):
        raise TypeError('level argument should be an int object.')

    # Prepare data
    chunks = []
    chunk_sizes = []
    for chunk in samples:
        chunks.append(chunk)
        chunk_sizes.append(_nbytes(chunk))

    chunks = b''.join(chunks)
    if not chunks:
        raise ValueError("The samples are empty content, can't finalize dictionary.")

    # custom_dict_bytes: existing dictionary.
    # samples_bytes: samples be stored concatenated in a single flat buffer.
    # samples_size_list: a list of each sample's size.
    # dict_size: maximal size of the dictionary, in bytes.
    # compression_level: compression level expected to use in production.
    dict_content = _finalize_dict(zstd_dict.dict_content,
                                  chunks, chunk_sizes,
                                  dict_size, level)

    return _zstd.ZstdDict(dict_content)

zstd_support_multithread = get_param_bounds(True, nbWorkers) != (0, 0)

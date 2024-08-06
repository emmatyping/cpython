from _zstd import ZstdDict, _train_dict, ZstdCompressor
import _zstd
import glob

def _nbytes(dat):
    if isinstance(dat, (bytes, bytearray)):
        return len(dat)
    with memoryview(dat) as mv:
        return mv.nbytes

def train_dict(samples, dict_size):
    """Train a zstd dictionary, return a ZSTDDict object.

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

def read_bytes(file):
    with open(file, "rb") as f:
        data = f.read()
    return data

print(_zstd.zstd_version_info)
files = glob.glob("Modules/*.c")
print(len(files))
data = tuple(read_bytes(f) for f in files)[:10]
print(f"Starting data size: {sum(map(len, data))}")
data_bytes = b"".join(data)
c = ZstdCompressor()
c.compress(data_bytes, c.CONTINUE)
normal_size = len(c.flush(c.FLUSH_FRAME))
print(f"Normal compressed size: {normal_size}")
zdict =train_dict(data, 2**10 * 64)
print(zdict)
c_dict = ZstdCompressor(zstd_dict=zdict)
c_dict.compress(data_bytes, c_dict.CONTINUE)
compressed = c_dict.flush(c_dict.FLUSH_FRAME)
print(compressed)
dict_size = len(compressed)
print(f"Dict compressed size: {dict_size}")

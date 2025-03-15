import io
import random
import re
import os
import unittest
import zstd

with open(os.path.abspath(__file__), 'rb') as f:
    THIS_FILE_BYTES = f.read()
    THIS_FILE_BYTES = re.sub(rb'\r?\n', rb'\n', THIS_FILE_BYTES)
    THIS_FILE_STR = THIS_FILE_BYTES.decode('utf-8')

DECOMPRESSED_100_PLUS_32KB = b'a' * (100 + 32*1024)
COMPRESSED_100_PLUS_32KB = zstd.compress(DECOMPRESSED_100_PLUS_32KB)

# dict data
words = [b'red', b'green', b'yellow', b'black', b'withe', b'blue',
            b'lilac', b'purple', b'navy', b'glod', b'silver', b'olive',
            b'dog', b'cat', b'tiger', b'lion', b'fish', b'bird']
SAMPLES = []
for i in range(300):
    sample = [b'%s = %d' % (random.choice(words), random.randrange(100))
                for j in range(20)]
    sample = b'\n'.join(sample)

    SAMPLES.append(sample)
assert len(SAMPLES) > 10

TRAINED_DICT = zstd.train_dict(SAMPLES, 3*1024)
assert len(TRAINED_DICT.dict_content) <= 3*1024

class FunctionsTestCase(unittest.TestCase):

    def test_version(self):
        s = '.'.join((str(i) for i in zstd.zstd_version_info))
        self.assertEqual(s, zstd.zstd_version)

    def test_compressionLevel_values(self):
        self.assertIs(type(zstd.compressionLevel_values.default), int)
        self.assertIs(type(zstd.compressionLevel_values.min), int)
        self.assertIs(type(zstd.compressionLevel_values.max), int)
        self.assertLess(zstd.compressionLevel_values.min, zstd.compressionLevel_values.max)

    def test_roundtrip_default(self):
        raw_dat = THIS_FILE_BYTES[:len(THIS_FILE_BYTES)//6]
        dat1 = zstd.compress(raw_dat)
        dat2 = zstd.decompress(dat1)
        self.assertEqual(dat2, raw_dat)

    def test_roundtrip_level(self):
        raw_dat = THIS_FILE_BYTES[:len(THIS_FILE_BYTES)//6]
        _default, minv, maxv = zstd.compressionLevel_values

        for level in range(max(-20, minv), maxv+1):
            dat1 = zstd.compress(raw_dat, level)
            dat2 = zstd.decompress(dat1)
            self.assertEqual(dat2, raw_dat)

    def test_get_frame_info(self):
        # no dict
        info = zstd.get_frame_info(COMPRESSED_100_PLUS_32KB[:20])
        self.assertEqual(info.decompressed_size, 32*1024+100)
        self.assertEqual(info.dictionary_id, 0)

        # use dict
        dat = zstd.compress(b'a'*345, zstd_dict=TRAINED_DICT)
        info = zstd.get_frame_info(dat)
        self.assertEqual(info.decompressed_size, 345)
        self.assertEqual(info.dictionary_id, TRAINED_DICT.dict_id)

        with self.assertRaisesRegex(zstd.ZstdError,
                                    'not less than the frame header'):
            zstd.get_frame_info(b'aaaaaaaaaaaaaa')

    def test_get_frame_size(self):
        size = zstd.get_frame_size(COMPRESSED_100_PLUS_32KB)
        self.assertEqual(size, len(COMPRESSED_100_PLUS_32KB))

        with self.assertRaisesRegex(zstd.ZstdError,
                                    'not less than this complete frame'):
            zstd.get_frame_size(b'aaaaaaaaaaaaaa')

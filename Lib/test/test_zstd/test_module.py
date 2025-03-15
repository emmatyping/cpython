import io
import pickle
import random
import re
import os
import unittest
import _compression
import zstd
from zstd import zstdfile

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


class ClassShapeTestCase(unittest.TestCase):

    def test_ZstdCompressor(self):
        # class attributes
        zstd.ZstdCompressor.CONTINUE
        zstd.ZstdCompressor.FLUSH_BLOCK
        zstd.ZstdCompressor.FLUSH_FRAME

        # method & member
        zstd.ZstdCompressor()
        zstd.ZstdCompressor(12, zstd_dict=TRAINED_DICT)
        c = zstd.ZstdCompressor(level=2, zstd_dict=TRAINED_DICT)

        c.compress(b'123456')
        c.compress(b'123456', zstd.ZstdCompressor.CONTINUE)
        c.compress(data=b'123456', mode=c.CONTINUE)

        c.flush()
        c.flush(zstd.ZstdCompressor.FLUSH_BLOCK)
        c.flush(mode=c.FLUSH_FRAME)

        c.last_mode

        # decompressor method & member
        with self.assertRaises(AttributeError):
            c.decompress(b'')
        with self.assertRaises(AttributeError):
            c.at_frame_edge
        with self.assertRaises(AttributeError):
            c.eof
        with self.assertRaises(AttributeError):
            c.needs_input

        # read only attribute
        with self.assertRaises(AttributeError):
            c.last_mode = zstd.ZstdCompressor.FLUSH_BLOCK

        # name
        self.assertIn('.ZstdCompressor', str(type(c)))

        # doesn't support pickle
        with self.assertRaises(TypeError):
            pickle.dumps(c)

        # supports subclass
        class SubClass(zstd.ZstdCompressor):
            pass

    def test_Decompressor(self):
        # method & member
        zstd.ZstdDecompressor()
        zstd.ZstdDecompressor(TRAINED_DICT, {})
        d = zstd.ZstdDecompressor(zstd_dict=TRAINED_DICT, options={})

        d.decompress(b'')
        d.decompress(b'', 100)
        d.decompress(data=b'', max_length = 100)

        d.eof
        d.needs_input
        d.unused_data

        # zstd.ZstdCompressor attributes
        with self.assertRaises(AttributeError):
            d.CONTINUE
        with self.assertRaises(AttributeError):
            d.FLUSH_BLOCK
        with self.assertRaises(AttributeError):
            d.FLUSH_FRAME
        with self.assertRaises(AttributeError):
            d.compress(b'')
        with self.assertRaises(AttributeError):
            d.flush()

        # zstd.EndlessZstdDecompressor attribute
        with self.assertRaises(AttributeError):
            d.at_frame_edge

        # read only attributes
        with self.assertRaises(AttributeError):
            d.eof = True
        with self.assertRaises(AttributeError):
            d.needs_input = True
        with self.assertRaises(AttributeError):
            d.unused_data = b''

        # name
        self.assertIn('.ZstdDecompressor', str(type(d)))

        # doesn't support pickle
        with self.assertRaises(TypeError):
            pickle.dumps(d)

        # supports subclass
        class SubClass(zstd.ZstdDecompressor):
            pass

    def test_EndlessDecompressor(self):
        # method & member
        zstd.EndlessZstdDecompressor(TRAINED_DICT, {})
        zstd.EndlessZstdDecompressor(zstd_dict=TRAINED_DICT, options={})
        d = zstd.EndlessZstdDecompressor()

        d.decompress(b'')
        d.decompress(b'', 100)
        d.decompress(data=b'', max_length = 100)

        d.at_frame_edge
        d.needs_input

        # zstd.ZstdCompressor attributes
        with self.assertRaises(AttributeError):
            zstd.EndlessZstdDecompressor.CONTINUE
        with self.assertRaises(AttributeError):
            zstd.EndlessZstdDecompressor.FLUSH_BLOCK
        with self.assertRaises(AttributeError):
            zstd.EndlessZstdDecompressor.FLUSH_FRAME
        with self.assertRaises(AttributeError):
            d.compress(b'')
        with self.assertRaises(AttributeError):
            d.flush()

        # zstd.ZstdDecompressor attributes
        with self.assertRaises(AttributeError):
            d.eof
        with self.assertRaises(AttributeError):
            d.unused_data

        # read only attributes
        with self.assertRaises(AttributeError):
            d.needs_input = True

        with self.assertRaises(AttributeError):
            d.at_frame_edge = True

        # name
        self.assertIn('.EndlessZstdDecompressor', str(type(d)))

        # doesn't support pickle
        with self.assertRaises(TypeError):
            pickle.dumps(d)

        # supports subclass
        class SubClass(zstd.EndlessZstdDecompressor):
            pass

    def test_ZstdDict(self):
        zstd.ZstdDict(b'12345678', True)
        zd = zstd.ZstdDict(b'12345678', is_raw=True)

        self.assertEqual(type(zd.dict_content), bytes)
        self.assertEqual(zd.dict_id, 0)
        self.assertEqual(zd.as_digested_dict[1], 0)
        self.assertEqual(zd.as_undigested_dict[1], 1)
        self.assertEqual(zd.as_prefix[1], 2)

        # name
        self.assertIn('.ZstdDict', str(type(zd)))

        # doesn't support pickle
        with self.assertRaisesRegex(TypeError,
                                    r'save \.dict_content attribute'):
            pickle.dumps(zd)
        with self.assertRaisesRegex(TypeError,
                                    r'save \.dict_content attribute'):
            pickle.dumps(zd.as_prefix)

        # supports subclass
        class SubClass(zstd.ZstdDict):
            pass

    def test_Strategy(self):
        # class attributes
        zstd.Strategy.fast
        zstd.Strategy.dfast
        zstd.Strategy.greedy
        zstd.Strategy.lazy
        zstd.Strategy.lazy2
        zstd.Strategy.btlazy2
        zstd.Strategy.btopt
        zstd.Strategy.btultra
        zstd.Strategy.btultra2

    def test_CParameter(self):
        zstd.CParameter.compressionLevel
        zstd.CParameter.windowLog
        zstd.CParameter.hashLog
        zstd.CParameter.chainLog
        zstd.CParameter.searchLog
        zstd.CParameter.minMatch
        zstd.CParameter.targetLength
        zstd.CParameter.strategy
        with self.assertRaises(NotImplementedError):
            zstd.CParameter.targetCBlockSize

        zstd.CParameter.enableLongDistanceMatching
        zstd.CParameter.ldmHashLog
        zstd.CParameter.ldmMinMatch
        zstd.CParameter.ldmBucketSizeLog
        zstd.CParameter.ldmHashRateLog

        zstd.CParameter.contentSizeFlag
        zstd.CParameter.checksumFlag
        zstd.CParameter.dictIDFlag

        zstd.CParameter.nbWorkers
        zstd.CParameter.jobSize
        zstd.CParameter.overlapLog

        t = zstd.CParameter.windowLog.bounds()
        self.assertEqual(len(t), 2)
        self.assertEqual(type(t[0]), int)
        self.assertEqual(type(t[1]), int)

    def test_DParameter(self):
        zstd.DParameter.windowLogMax

        t = zstd.DParameter.windowLogMax.bounds()
        self.assertEqual(len(t), 2)
        self.assertEqual(type(t[0]), int)
        self.assertEqual(type(t[1]), int)

    def test_zstderror_pickle(self):
        try:
            zstd.decompress(b'invalid data')
        except Exception as e:
            s = pickle.dumps(e)
            obj = pickle.loads(s)
            self.assertEqual(type(obj), zstd.ZstdError)
        else:
            self.assertFalse(True, 'unreachable code path')

    def test_ZstdFile_extend(self):
        # These classes and variables can be used to extend zstd.ZstdFile,
        # such as Seekablezstd.ZstdFile(zstd.ZstdFile), so pin them down.
        self.assertTrue(issubclass(zstd.ZstdFile, io.BufferedIOBase))
        self.assertIs(zstd.ZstdFile._READER_CLASS,
                      _compression.DecompressReader)

        # mode
        self.assertEqual(zstdfile._MODE_CLOSED, 0)
        self.assertEqual(zstdfile._MODE_READ, 1)
        self.assertEqual(zstdfile._MODE_WRITE, 2)

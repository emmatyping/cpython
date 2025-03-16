import io
import pickle
import random
import re
import os
import unittest
import _compression
from test.support.import_helper import import_module
from test.support import _1M

zstd = import_module("zstd")
from zstd import (
    zstdfile,
    compress,
    decompress,
    compress_stream,
    decompress_stream,
    ZstdCompressor,
    ZstdDecompressor,
    EndlessZstdDecompressor,
    ZstdDict,
    ZstdError,
    zstd_version,
    zstd_version_info,
    compressionLevel_values,
    get_frame_info,
    get_frame_size,
    finalize_dict,
    train_dict,
    CParameter,
    DParameter,
    Strategy,
    ZstdFile,
    zstd_support_multithread,
)

with open(os.path.abspath(__file__), "rb") as f:
    THIS_FILE_BYTES = f.read()
    THIS_FILE_BYTES = re.sub(rb"\r?\n", rb"\n", THIS_FILE_BYTES)
    THIS_FILE_STR = THIS_FILE_BYTES.decode("utf-8")

DECOMPRESSED_100_PLUS_32KB = b"a" * (100 + 32 * 1024)
COMPRESSED_100_PLUS_32KB = compress(DECOMPRESSED_100_PLUS_32KB)

# dict data
words = [
    b"red",
    b"green",
    b"yellow",
    b"black",
    b"withe",
    b"blue",
    b"lilac",
    b"purple",
    b"navy",
    b"glod",
    b"silver",
    b"olive",
    b"dog",
    b"cat",
    b"tiger",
    b"lion",
    b"fish",
    b"bird",
]
SAMPLES = []
for i in range(300):
    sample = [
        b"%s = %d" % (random.choice(words), random.randrange(100)) for j in range(20)
    ]
    sample = b"\n".join(sample)

    SAMPLES.append(sample)
assert len(SAMPLES) > 10

TRAINED_DICT = train_dict(SAMPLES, 3 * 1024)
assert len(TRAINED_DICT.dict_content) <= 3 * 1024


class FunctionsTestCase(unittest.TestCase):

    def test_version(self):
        s = ".".join((str(i) for i in zstd_version_info))
        self.assertEqual(s, zstd_version)

    def test_compressionLevel_values(self):
        self.assertIs(type(compressionLevel_values.default), int)
        self.assertIs(type(compressionLevel_values.min), int)
        self.assertIs(type(compressionLevel_values.max), int)
        self.assertLess(compressionLevel_values.min, compressionLevel_values.max)

    def test_roundtrip_default(self):
        raw_dat = THIS_FILE_BYTES[: len(THIS_FILE_BYTES) // 6]
        dat1 = compress(raw_dat)
        dat2 = decompress(dat1)
        self.assertEqual(dat2, raw_dat)

    def test_roundtrip_level(self):
        raw_dat = THIS_FILE_BYTES[: len(THIS_FILE_BYTES) // 6]
        _default, minv, maxv = compressionLevel_values

        for level in range(max(-20, minv), maxv + 1):
            dat1 = compress(raw_dat, level)
            dat2 = decompress(dat1)
            self.assertEqual(dat2, raw_dat)

    def test_get_frame_info(self):
        # no dict
        info = get_frame_info(COMPRESSED_100_PLUS_32KB[:20])
        self.assertEqual(info.decompressed_size, 32 * 1024 + 100)
        self.assertEqual(info.dictionary_id, 0)

        # use dict
        dat = compress(b"a" * 345, zstd_dict=TRAINED_DICT)
        info = get_frame_info(dat)
        self.assertEqual(info.decompressed_size, 345)
        self.assertEqual(info.dictionary_id, TRAINED_DICT.dict_id)

        with self.assertRaisesRegex(ZstdError, "not less than the frame header"):
            get_frame_info(b"aaaaaaaaaaaaaa")

    def test_get_frame_size(self):
        size = get_frame_size(COMPRESSED_100_PLUS_32KB)
        self.assertEqual(size, len(COMPRESSED_100_PLUS_32KB))

        with self.assertRaisesRegex(ZstdError, "not less than this complete frame"):
            get_frame_size(b"aaaaaaaaaaaaaa")


class ClassShapeTestCase(unittest.TestCase):

    def test_ZstdCompressor(self):
        # class attributes
        ZstdCompressor.CONTINUE
        ZstdCompressor.FLUSH_BLOCK
        ZstdCompressor.FLUSH_FRAME

        # method & member
        ZstdCompressor()
        ZstdCompressor(12, zstd_dict=TRAINED_DICT)
        c = ZstdCompressor(level=2, zstd_dict=TRAINED_DICT)

        c.compress(b"123456")
        c.compress(b"123456", ZstdCompressor.CONTINUE)
        c.compress(data=b"123456", mode=c.CONTINUE)

        c.flush()
        c.flush(ZstdCompressor.FLUSH_BLOCK)
        c.flush(mode=c.FLUSH_FRAME)

        c.last_mode

        # decompressor method & member
        with self.assertRaises(AttributeError):
            c.decompress(b"")
        with self.assertRaises(AttributeError):
            c.at_frame_edge
        with self.assertRaises(AttributeError):
            c.eof
        with self.assertRaises(AttributeError):
            c.needs_input

        # read only attribute
        with self.assertRaises(AttributeError):
            c.last_mode = ZstdCompressor.FLUSH_BLOCK

        # name
        self.assertIn(".ZstdCompressor", str(type(c)))

        # doesn't support pickle
        with self.assertRaises(TypeError):
            pickle.dumps(c)

        # supports subclass
        class SubClass(ZstdCompressor):
            pass

    def test_Decompressor(self):
        # method & member
        ZstdDecompressor()
        ZstdDecompressor(TRAINED_DICT, {})
        d = ZstdDecompressor(zstd_dict=TRAINED_DICT, options={})

        d.decompress(b"")
        d.decompress(b"", 100)
        d.decompress(data=b"", max_length=100)

        d.eof
        d.needs_input
        d.unused_data

        # ZstdCompressor attributes
        with self.assertRaises(AttributeError):
            d.CONTINUE
        with self.assertRaises(AttributeError):
            d.FLUSH_BLOCK
        with self.assertRaises(AttributeError):
            d.FLUSH_FRAME
        with self.assertRaises(AttributeError):
            d.compress(b"")
        with self.assertRaises(AttributeError):
            d.flush()

        # EndlessZstdDecompressor attribute
        with self.assertRaises(AttributeError):
            d.at_frame_edge

        # read only attributes
        with self.assertRaises(AttributeError):
            d.eof = True
        with self.assertRaises(AttributeError):
            d.needs_input = True
        with self.assertRaises(AttributeError):
            d.unused_data = b""

        # name
        self.assertIn(".ZstdDecompressor", str(type(d)))

        # doesn't support pickle
        with self.assertRaises(TypeError):
            pickle.dumps(d)

        # supports subclass
        class SubClass(ZstdDecompressor):
            pass

    def test_EndlessDecompressor(self):
        # method & member
        EndlessZstdDecompressor(TRAINED_DICT, {})
        EndlessZstdDecompressor(zstd_dict=TRAINED_DICT, options={})
        d = EndlessZstdDecompressor()

        d.decompress(b"")
        d.decompress(b"", 100)
        d.decompress(data=b"", max_length=100)

        d.at_frame_edge
        d.needs_input

        # ZstdCompressor attributes
        with self.assertRaises(AttributeError):
            EndlessZstdDecompressor.CONTINUE
        with self.assertRaises(AttributeError):
            EndlessZstdDecompressor.FLUSH_BLOCK
        with self.assertRaises(AttributeError):
            EndlessZstdDecompressor.FLUSH_FRAME
        with self.assertRaises(AttributeError):
            d.compress(b"")
        with self.assertRaises(AttributeError):
            d.flush()

        # ZstdDecompressor attributes
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
        self.assertIn(".EndlessZstdDecompressor", str(type(d)))

        # doesn't support pickle
        with self.assertRaises(TypeError):
            pickle.dumps(d)

        # supports subclass
        class SubClass(EndlessZstdDecompressor):
            pass

    def test_ZstdDict(self):
        ZstdDict(b"12345678", True)
        zd = ZstdDict(b"12345678", is_raw=True)

        self.assertEqual(type(zd.dict_content), bytes)
        self.assertEqual(zd.dict_id, 0)
        self.assertEqual(zd.as_digested_dict[1], 0)
        self.assertEqual(zd.as_undigested_dict[1], 1)
        self.assertEqual(zd.as_prefix[1], 2)

        # name
        self.assertIn(".ZstdDict", str(type(zd)))

        # doesn't support pickle
        with self.assertRaisesRegex(TypeError, r"save \.dict_content attribute"):
            pickle.dumps(zd)
        with self.assertRaisesRegex(TypeError, r"save \.dict_content attribute"):
            pickle.dumps(zd.as_prefix)

        # supports subclass
        class SubClass(ZstdDict):
            pass

    def test_Strategy(self):
        # class attributes
        Strategy.fast
        Strategy.dfast
        Strategy.greedy
        Strategy.lazy
        Strategy.lazy2
        Strategy.btlazy2
        Strategy.btopt
        Strategy.btultra
        Strategy.btultra2

    def test_CParameter(self):
        CParameter.compressionLevel
        CParameter.windowLog
        CParameter.hashLog
        CParameter.chainLog
        CParameter.searchLog
        CParameter.minMatch
        CParameter.targetLength
        CParameter.strategy
        with self.assertRaises(NotImplementedError):
            CParameter.targetCBlockSize

        CParameter.enableLongDistanceMatching
        CParameter.ldmHashLog
        CParameter.ldmMinMatch
        CParameter.ldmBucketSizeLog
        CParameter.ldmHashRateLog

        CParameter.contentSizeFlag
        CParameter.checksumFlag
        CParameter.dictIDFlag

        CParameter.nbWorkers
        CParameter.jobSize
        CParameter.overlapLog

        t = CParameter.windowLog.bounds()
        self.assertEqual(len(t), 2)
        self.assertEqual(type(t[0]), int)
        self.assertEqual(type(t[1]), int)

    def test_DParameter(self):
        DParameter.windowLogMax

        t = DParameter.windowLogMax.bounds()
        self.assertEqual(len(t), 2)
        self.assertEqual(type(t[0]), int)
        self.assertEqual(type(t[1]), int)

    def test_zstderror_pickle(self):
        try:
            decompress(b"invalid data")
        except Exception as e:
            s = pickle.dumps(e)
            obj = pickle.loads(s)
            self.assertEqual(type(obj), ZstdError)
        else:
            self.assertFalse(True, "unreachable code path")

    def test_ZstdFile_extend(self):
        # These classes and variables can be used to extend ZstdFile,
        # such as SeekableZstdFile(ZstdFile), so pin them down.
        self.assertTrue(issubclass(ZstdFile, io.BufferedIOBase))
        self.assertIs(ZstdFile._READER_CLASS, _compression.DecompressReader)

        # mode
        self.assertEqual(zstdfile._MODE_CLOSED, 0)
        self.assertEqual(zstdfile._MODE_READ, 1)
        self.assertEqual(zstdfile._MODE_WRITE, 2)


class CompressorTestCase(unittest.TestCase):

    def test_simple_bad_args(self):
        # ZstdCompressor
        self.assertRaises(TypeError, ZstdCompressor, [])
        self.assertRaises(TypeError, ZstdCompressor, level=3.14)
        self.assertRaises(TypeError, ZstdCompressor, level="abc")
        self.assertRaises(TypeError, ZstdCompressor, options=b"abc")

        self.assertRaises(TypeError, ZstdCompressor, zstd_dict=123)
        self.assertRaises(TypeError, ZstdCompressor, zstd_dict=b"abcd1234")
        self.assertRaises(TypeError, ZstdCompressor, zstd_dict={1: 2, 3: 4})

        with self.assertRaises(ValueError):
            ZstdCompressor(2**31)
        with self.assertRaises(ValueError):
            ZstdCompressor(options={2**31: 100})

        with self.assertRaises(ZstdError):
            ZstdCompressor(options={CParameter.windowLog: 100})
        with self.assertRaises(ZstdError):
            ZstdCompressor(options={3333: 100})

        # Method bad arguments
        zc = ZstdCompressor()
        self.assertRaises(TypeError, zc.compress)
        self.assertRaises((TypeError, ValueError), zc.compress, b"foo", b"bar")
        self.assertRaises(TypeError, zc.compress, "str")
        self.assertRaises((TypeError, ValueError), zc.flush, b"foo")
        self.assertRaises(TypeError, zc.flush, b"blah", 1)

        self.assertRaises(ValueError, zc.compress, b'', -1)
        self.assertRaises(ValueError, zc.compress, b'', 3)
        self.assertRaises(ValueError, zc.flush, zc.CONTINUE) # 0
        self.assertRaises(ValueError, zc.flush, 3)

        zc.compress(b'')
        zc.compress(b'', zc.CONTINUE)
        zc.compress(b'', zc.FLUSH_BLOCK)
        zc.compress(b'', zc.FLUSH_FRAME)
        empty = zc.flush()
        zc.flush(zc.FLUSH_BLOCK)
        zc.flush(zc.FLUSH_FRAME)

    def test_compress_parameters(self):
        d = {CParameter.compressionLevel : 10,

             CParameter.windowLog : 12,
             CParameter.hashLog : 10,
             CParameter.chainLog : 12,
             CParameter.searchLog : 12,
             CParameter.minMatch : 4,
             CParameter.targetLength : 12,
             CParameter.strategy : Strategy.lazy,

             CParameter.enableLongDistanceMatching : 1,
             CParameter.ldmHashLog : 12,
             CParameter.ldmMinMatch : 11,
             CParameter.ldmBucketSizeLog : 5,
             CParameter.ldmHashRateLog : 12,

             CParameter.contentSizeFlag : 1,
             CParameter.checksumFlag : 1,
             CParameter.dictIDFlag : 0,

             CParameter.nbWorkers : 2 if zstd_support_multithread else 0,
             CParameter.jobSize : 5*_1M if zstd_support_multithread else 0,
             CParameter.overlapLog : 9 if zstd_support_multithread else 0,
             }
        ZstdCompressor(options=d)

        # larger than signed int, ValueError
        d1 = d.copy()
        d1[CParameter.ldmBucketSizeLog] = 2**31
        self.assertRaises(ValueError, ZstdCompressor, d1)

        # clamp compressionLevel
        compress(b'', compressionLevel_values.max+1)
        compress(b'', compressionLevel_values.min-1)

        compress(b'', {CParameter.compressionLevel:compressionLevel_values.max+1})
        compress(b'', {CParameter.compressionLevel:compressionLevel_values.min-1})

        # zstd lib doesn't support MT compression
        if not zstd_support_multithread:
            with self.assertRaises(ZstdError):
                ZstdCompressor({CParameter.nbWorkers:4})
            with self.assertRaises(ZstdError):
                ZstdCompressor({CParameter.jobSize:4})
            with self.assertRaises(ZstdError):
                ZstdCompressor({CParameter.overlapLog:4})

        # out of bounds error msg
        option = {CParameter.windowLog:100}
        with self.assertRaisesRegex(ZstdError,
                (r'Error when setting zstd compression parameter "windowLog", '
                 r'it should \d+ <= value <= \d+, provided value is 100\. '
                 r'\(zstd v\d\.\d\.\d, (?:32|64)-bit build\)')):
            compress(b'', option)

    def test_unknown_compression_parameter(self):
        KEY = 100001234
        option = {CParameter.compressionLevel: 10,
                  KEY: 200000000}
        pattern = r'Zstd compression parameter.*?"unknown parameter \(key %d\)"' \
                  % KEY
        with self.assertRaisesRegex(ZstdError, pattern):
            ZstdCompressor(option)

    @unittest.skipIf(not zstd_support_multithread,
                     "zstd build doesn't support multi-threaded compression")
    def test_zstd_multithread_compress(self):
        size = 40*_1M
        b = THIS_FILE_BYTES * (size // len(THIS_FILE_BYTES))

        options = {CParameter.compressionLevel : 4,
                  CParameter.nbWorkers : 2}

        # compress()
        dat1 = compress(b, options=options)
        dat2 = decompress(dat1)
        self.assertEqual(dat2, b)

        # ZstdCompressor
        c = ZstdCompressor(options=options)
        dat1 = c.compress(b, c.CONTINUE)
        dat2 = c.compress(b, c.FLUSH_BLOCK)
        dat3 = c.compress(b, c.FLUSH_FRAME)
        dat4 = decompress(dat1+dat2+dat3)
        self.assertEqual(dat4, b * 3)

        # ZstdFile
        with ZstdFile(io.BytesIO(), 'w',
                      options=options) as f:
            f.write(b)

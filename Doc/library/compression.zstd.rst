:mod:`!compression.zstd` --- Compression compatible with the Zstandard format
=============================================================================

.. module:: compression.zstd
   :synopsis: Low level interface to compression and decompression routines in Meta's zstd library

.. versionadded:: 3.14

.. moduleauthor:: Emma Harper Smith <emma@emmatyping.dev>
.. sectionauthor:: Emma Harper Smith <emma@emmatyping.dev>

**Source code:** :source:`Lib/compression/zstd/__init__.py`

--------------

This module provides classes and convenience functions for compressing and
decompressing data using the Zstandard (or ``zstd``) compression algorithm. Also
included is a file interface supporting reading and writing contents of ``.zst``
files created from the :program:`zstd` utility, as well as raw compressed streams.

The interface provided by this module is very similar to that of the :mod:`bz2`
module. Note that :class:`ZstdFile` and :class:`bz2.BZ2File` are *not*
thread-safe, so if you need to use a single :class:`ZstdFile` instance
from multiple threads, it is necessary to protect it with a lock.


.. exception:: ZstdError

   This exception is raised when an error occurs during compression or
   decompression, or while initializing the compressor/decompressor state.


Reading and writing compressed files
------------------------------------

.. function:: open(filename, mode="rb", *, format=None, check=-1, preset=None, filters=None, encoding=None, errors=None, newline=None)

   Open an Zstandard-compressed file in binary or text mode, returning a
   :term:`file object`.

   The *filename* argument can be either an actual file name (given as a
   :class:`str`, :class:`bytes` or :term:`path-like <path-like object>` object),
   in which case the named file is opened, or it can be an existing file object
   to read from or write to.

   The mode argument can be either 'r' for reading (default), 'w' for
   overwriting, or 'x' for exclusive creation. These can equivalently be given
   as 'rb', 'wb', and 'xb' respectively. You may also open in text mode with
   'rt', 'wt', and 'xt' respectively.

   When opening a file for reading, the *options* argument can be a dictionary
   providing advanced decompression parameters, see :class:`DParameter` for
   detailed information about supported parameters. The *zstd_dict* argument
   is a :class:`ZstdDict` instance to be used during decompression. When
   opening a file for reading, the *level* argument should not be used.

   When opening a file for writing, the *options* argument can be a dictionary
   providing advanced decompression parameters, see :class:`CParameter` for
   detailed information about supported parameters. The *level* argument is the
   compression level to use when writing compressed data. Only one
   of *level* or *options* may be passed. The *zstd_dict* argument
   is a :class:`ZstdDict` instance to be used during compression.

   For binary mode, this function is equivalent to the :class:`ZstdFile`
   constructor: ``ZstdFile(filename, mode, ...)``. In this case, the
   *encoding*, *errors* and *newline* parameters must not be provided.

   For text mode, a :class:`ZstdFile` object is created, and wrapped in an
   :class:`io.TextIOWrapper` instance with the specified encoding, error handling
   behavior, and line ending(s).

.. class:: ZstdFile(filename, mode="r", *, level=None, options=None, zstd_dict=None)

   Open a Zstandard-compressed file in binary mode.

   An :class:`ZstdFile` can wrap an already-open :term:`file object`, or operate
   directly on a named file. The *filename* argument specifies either the file
   object to wrap, or the name of the file to open (as a :class:`str`,
   :class:`bytes` or :term:`path-like <path-like object>` object). When wrapping an
   existing file object, the wrapped file will not be closed when the
   :class:`ZstdFile` is closed.

   The *mode* argument can be either ``"r"`` for reading (default), ``"w"`` for
   overwriting, ``"x"`` for exclusive creation, or ``"a"`` for appending. These
   can equivalently be given as ``"rb"``, ``"wb"``, ``"xb"`` and ``"ab"``
   respectively.

   If *filename* is a file object (rather than an actual file name), a mode of
   ``"w"`` does not truncate the file, and is instead equivalent to ``"a"``.

   When opening a file for reading, the input file may be the concatenation of
   multiple separate Zstandard frames. These are transparently decoded as a
   single logical stream.

   When opening a file for reading, the *options* and *zstd_dict* arguments
   have the same meanings as for :class:`ZstdDecompressor`. In this case, the
   *level* argument should not be used.

   When opening a file for writing, the *options*, *zstd_dict* and *level*
   arguments have the same meanings as for :class:`ZstdCompressor`.

   :class:`ZstdFile` supports all the members specified by
   :class:`io.BufferedIOBase`, except for :meth:`~io.BufferedIOBase.detach`
   and :meth:`~io.IOBase.truncate`.
   Iteration and the :keyword:`with` statement are supported.

   The following method and attributes are also provided:

   .. method:: peek(size=-1)

      Return buffered data without advancing the file position. At least one
      byte of data will be returned, unless EOF has been reached. The exact
      number of bytes returned is unspecified (the *size* argument is ignored).

      .. note:: While calling :meth:`peek` does not change the file position of
         the :class:`ZstdFile`, it may change the position of the underlying
         file object (e.g. if the :class:`ZstdFile` was constructed by passing a
         file object for *filename*).

   .. attribute:: mode

      ``'rb'`` for reading and ``'wb'`` for writing.


   .. attribute:: name

      The zstd file name. Equivalent to the :attr:`~io.FileIO.name`
      attribute of the underlying :term:`file object`.

Compressing and decompressing data in memory
--------------------------------------------

.. class:: ZstdCompressor(level=None, options=None, zstd_dict=None)

   Create a compressor object, which can be used to compress data incrementally.

   For a more convenient way of compressing a single chunk of data, see
   :func:`compress`.

   The *level* argument is an integer from 1 to 22 controlling the level of
   compression. Levels 20 and above are considered "ultra" and take more memory.
   Negative compression levels improve compression speed at the cost of
   compression ratio. If advanced compression options are needed, this argument
   must be omitted and in the *options* dictionary the
   :attr:`~CParameter.compressionLevel` parameter should be set.

   The *options* argument is a Python dictionary containing advanced compression
   parameters. The valid keys and values for compression parameters are
   documented as part of the :class:`CParameter` documentation.

   The *zstd_dict* argument is an instance of :class:`ZstdDict`, a Zstandard
   dictionary, containing trained data to improve compression efficiency. The
   function :func:`train_dict` can be used to generate a Zstandard dictionary.

   .. attribute:: CONTINUE

      Collect more data for compression, which may or may not generate output
      immediately. This mode optimizes the compression ratio by maximizing the
      amount of data per block and frame.

   .. attribute:: FLUSH_BLOCK

      Complete and write a block to the data stream. The data returned so far
      can be immediately decompressed. Past data can still be referenced in
      future blocks generated by calls to :meth:`ZstdCompressor.compress`,
      improving compression.

   .. attribute:: FLUSH_FRAME

      Complete and write out a frame. Future data provided to
      :meth:`ZstdCompressor.compress` will be written into a new frame and
      *cannot* reference past data.

   .. method:: compress(data, mode=ZstdCompressor.CONTINUE)

      Compress *data* (a :term:`bytes-like object`), returning a :class:`bytes`
      object if possible, or an empty byte string otherwise. Some of *data* may
      be buffered internally, for use in later calls to :meth:`compress` and
      :meth:`flush`. The returned data should be concatenated with the output
      of any previous calls to :meth:`compress`.

      The *mode* argument is a :class:`ZstdCompressor` attribute, either
      :attr:`ZstdCompressor.CONTINUE`, :attr:`ZstdCompressor.FLUSH_BLOCK`,
      or :attr:`ZstdCompressor.FLUSH_FRAME`.

      When you have finished providing data to the compressor, call the
      :meth:`flush` method to finish the compression process.

   .. method:: flush(mode)

      Finish the compression process, returning a :class:`bytes` object
      containing any data stored in the compressor's internal buffers.

      The *mode* argument is a :class:`ZstdCompressor` attribute, either
      :attr:`ZstdCompressor.FLUSH_BLOCK`, or :attr:`ZstdCompressor.FLUSH_FRAME`.

      The compressor cannot be used after this method has been called.


.. class:: ZstdDecompressor(zstd_dict=None, options=None)

   Create a decompressor object, which can be used to decompress data
   incrementally.

   For a more convenient way of decompressing an entire compressed stream at
   once, see :func:`decompress`.

   The *options* argument is a Python dictionary containing advanced
   decompression parameters. The valid keys and values for compression
   parameters are documented as part of the :class:`DParameter` documentation.

   The *zstd_dict* argument is an instance of :class:`ZstdDict`, a Zstandard
   dictionary, containing trained data used during compression. This must be
   the same Zstandard dictionary used during compression.

   .. note::
      This class does not transparently handle inputs containing multiple
      compressed frames, unlike :func:`decompress` and :class:`ZstdFile`. To
      decompress a multi-stream input, you should use :func:`decompress` or
      :class:`ZstdFile` if working with a :term:`file object`.

   .. method:: decompress(data, max_length=-1)

      Decompress *data* (a :term:`bytes-like object`), returning
      uncompressed data as bytes. Some of *data* may be buffered
      internally, for use in later calls to :meth:`decompress`. The
      returned data should be concatenated with the output of any
      previous calls to :meth:`decompress`.

      If *max_length* is nonnegative, returns at most *max_length*
      bytes of decompressed data. If this limit is reached and further
      output can be produced, the :attr:`~.needs_input` attribute will
      be set to ``False``. In this case, the next call to
      :meth:`~.decompress` may provide *data* as ``b''`` to obtain
      more of the output.

      If all of the input data was decompressed and returned (either
      because this was less than *max_length* bytes, or because
      *max_length* was negative), the :attr:`~.needs_input` attribute
      will be set to ``True``.

      Attempting to decompress data after the end of stream is reached
      raises an :exc:`EOFError`. Any data found after the end of the
      stream is ignored and saved in the :attr:`~.unused_data` attribute.

   .. attribute:: eof

      ``True`` if the end-of-stream marker has been reached.

   .. attribute:: unused_data

      Data found after the end of the compressed stream.

      Before the end of the stream is reached, this will be ``b""``.

   .. attribute:: needs_input

      ``False`` if the :meth:`.decompress` method can provide more
      decompressed data before requiring new uncompressed input.


.. function:: compress(data, level=None, options=None, zstd_dict=None)

   Compress *data* (a :term:`bytes-like object`), returning the compressed
   data as a :class:`bytes` object.

   See :class:`ZstdCompressor` above for a description of the *level*,
   *options*, and *zstd_dict* arguments.


.. function:: decompress(data, zstd_dict=None, options=None)

   Decompress *data* (a :term:`bytes-like object`), returning the uncompressed
   data as a :class:`bytes` object.

   If *data* is the concatenation of multiple distinct compressed frames,
   decompress all of these frames, and return the concatenation of the results.

   See :class:`ZstdDecompressor` above for a description of the *options*, and
   *zstd_dict* arguments.

Zstandard Dictionaries
----------------------


.. function:: train_dict(samples, dict_size)

   Train a Zstandard dictionary, returning a :class:`ZstdDict` instance.
   Zstandard dictionaries enable more efficient compression of smaller sizes
   of data, which is traditionally difficult to compress due to less repetition.
   If you are compressing multiple similar groups of data (such as similar
   files), Zstandard dictionaries can improve compression ratios and speed
   significantly.

   The *samples* argument (an iterable of :class:`bytes`), is the population of
   samples used to train the Zstandard dictionary.

   The *dict_size* argument, an integer, is the maximum size (in bytes) the
   Zstandard dictionary should be. The Zstandard documentation suggests an
   absolute maximum of no more than 100KB, but the maximum can often be smaller
   depending on the data. Larger dictionaries generally slow down compression,
   but improve compression ratios. Smaller dictionaries lead to faster
   compression, but reduce the compression ratio.


.. function:: finalize_dict(zstd_dict, samples, dict_size, level)

   An advanced function for converting a "raw content" Zstandard dictionary into
   a regular Zstandard dictionary. "Raw content" dictionaries are a sequence of
   bytes that do not need to follow the structure of a normal Zstandard
   dictionary.

   The *zstd_dict* argument is a :class:`ZstdDict` instance with
   attribute ``dict_contents`` containing the raw dictionary content.

   The *samples* argument (an iterable of bytes), contains sample data for
   generating the Zstandard dictionary.

   The *dict_size* argument, an integer, is the maximum size (in bytes) the
   Zstandard dictionary should be. Please see :func:`train_dict` for
   suggestions on the maximum dictionary size.

   The *level* argument (an integer) is the compression level expected to be
   passed to the compressors using this dictionary. The dictionary information
   varies for each compression level, so tuning for the proper compression
   level can make compression more efficient.


.. class:: ZstdDict(dict_content, is_raw=False)

   Initialize a Zstandard dictionary. Use :func:`train_dict` if you need to
   train a new dictionary from sample data.

   The *dict_content* argument (a :term:`bytes-like object`), is the already
   trained dictionary information.

   The *is_raw* argument, a boolean, is an advanced parameter. ``True`` means
   *dict_content* is a "raw content" dictionary, without any format
   restrictions. ``False`` means *dict_content* is an ordinary Zstandard
   dictionary, created from Zstandard functions, e.g. :func:`train_dict` or the
   ``zstd`` CLI.

    .. attribute:: dict_content

        The content of the Zstandard dictionary, a ``bytes`` object. It's the
        same as *dict_content* argument in :meth:`~ZstdDict.__init__` method.
        It can be used with other programs, such as the ``zstd`` CLI program.

    .. attribute:: dict_id

        Identifier of the Zstandard dictionary, a 32-bit unsigned integer value.

        Non-zero means the dictionary is ordinary, created by Zstandard
        functions and following the Zstandard format.

        ``0`` means a "raw content" dictionary, free of any format restriction,
        used for advanced users.

        .. note::

            The meaning of ``0`` for :attr:`ZstdDict.dict_id` is different from
            the ``dictionary_id`` argument to the :func:`get_frame_info`
            function.

    .. attribute:: as_digested_dict

        Load as a digested dictionary, see below.

    .. attribute:: as_undigested_dict

        Load as an undigested dictionary.

        Digesting dictionary is a costly operation. These two attributes can
        control how the dictionary is loaded to the compressor, by passing them
        as the ``zstd_dict`` argument, e.g.
        ``compress(data, zstd_dict=zd.as_digested_dict)``.

        If don't use one of these attributes, an **undigested** dictionary is
        passed by default.

        .. list-table:: Difference for compression
            :widths: 12 12 12
            :header-rows: 1

            * -
              - | Digested
                | dictionary
              - | Undigested
                | dictionary
            * - | Some advanced
                | parameters of the
                | compressor may
                | be overridden
                | by dictionary's
                | parameters
              - | ``windowLog``, ``hashLog``,
                | ``chainLog``, ``searchLog``,
                | ``minMatch``, ``targetLength``,
                | ``strategy``,
                | ``enableLongDistanceMatching``,
                | ``ldmHashLog``, ``ldmMinMatch``,
                | ``ldmBucketSizeLog``,
                | ``ldmHashRateLog``, and some
                | non-public parameters.
              - No
            * - | ZstdDict internally
                | caches the dictionary
              - | Yes. It's faster when
                | loading a digested
                | dictionary again with the same
                | compression level.
              - | No. If load an undigested
                | dictionary multiple times,
                | consider reusing a
                | compressor object.

        For decompression, they have the same effect. A **digested** dictionary
        is used for decompression by default, which is faster when loaded
        multiple times.


Miscellaneous
-------------

.. function:: get_frame_info(frame_buffer)

   Retrieve a :class:`named_tuple` containing information about the provided
   frame. The current elements are

   * ``decompressed_size``: The size of the decompressed contents of the frame.

   * ``dictionary_id``: a 32-bit unsigned integer value. ``0`` means the
     dictionary ID was not recorded in the frame header, the frame may or
     may not need a dictionary to be decoded, or the ID of such a dictionary is
     not specified.

   More elements may be added to the named tuple in the future.

.. attribute:: zstd_support_multithread

   This attribute can be used to detect if the underlying Zstandard library
   supports multi-threading. If this attribute is ``False`` and a function
   attempts to set :attr:`CParameter.nbWorkers` to anything other than ``0``,
   an exception will be raised.

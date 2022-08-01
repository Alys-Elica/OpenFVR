# This is a generated file! Please edit source .ksy file and use kaitai-struct-compiler to rebuild

import kaitaistruct
from kaitaistruct import KaitaiStruct, KaitaiStream, BytesIO
from uncompress import Uncompress

if getattr(kaitaistruct, 'API_VERSION', (0, 9)) < (0, 9):
    raise Exception("Incompatible Kaitai Struct Python API: 0.9 or later is required, but you have %s" % (kaitaistruct.__version__))

class Pak(KaitaiStruct):
    def __init__(self, _io, _parent=None, _root=None):
        self._io = _io
        self._parent = _parent
        self._root = _root if _root else self
        self._read()

    def _read(self):
        self.header = self._io.read_bytes(4)
        if not self.header == b"\x50\x41\x4B\x46":
            raise kaitaistruct.ValidationNotEqualError(b"\x50\x41\x4B\x46", self.header, self._io, u"/seq/0")
        self.file_size = self._io.read_u4le()
        self.files = []
        i = 0
        while not self._io.is_eof():
            self.files.append(Pak.File(self._io, self, self._root))
            i += 1

    class File(KaitaiStruct):
        def __init__(self, _io, _parent=None, _root=None):
            self._io = _io
            self._parent = _parent
            self._root = _root if _root else self
            self._read()

        def _read(self):
            self.name = (KaitaiStream.bytes_terminate(self._io.read_bytes(16), 0, False)).decode(u"UTF-8")
            self.compression_level = self._io.read_u4le()
            self.len_data = self._io.read_u4le()
            self.len_data_uncompressed = self._io.read_u4le()
            self._raw_data = self._io.read_bytes(self.len_data)
            _process = Uncompress(self.compression_level)
            self.data = _process.decode(self._raw_data)

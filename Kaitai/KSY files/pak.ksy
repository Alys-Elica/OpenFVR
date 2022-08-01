meta:
  id: pak
  title: PAK file
  file-extension: pak
  endian: le
  bit-endian: le
seq:
  - id: header
    contents: "PAKF"
  - id: file_size
    type: u4
  - id: files
    type: file
    repeat: eos
types:
  file:
    seq:
      - id: name
        type: str
        encoding: UTF-8
        size: 16
        terminator: 0
      - id: compression_level
        type: u4
      - id: len_data
        type: u4
      - id: len_data_uncompressed
        type: u4
      - id: data
        size: len_data
        process: uncompress(compression_level)

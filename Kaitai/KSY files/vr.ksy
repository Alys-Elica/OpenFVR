meta:
  id: vr
  title: VR file
  file-extension: vr
  endian: le
  bit-endian: le
seq:
  - id: header
    contents: [0xAB, 0x84, 0xFA, 0x12]
  - id: file_size
    type: u4
  - id: type
    type: u4
  - id: main_image_size
    type: u4
  - id: main_image_data
    type: image
  - id: animations
    type: animation
types:
  image:
    seq:
      - id: quality
        type: u4
      - id: data_size
        type: u4
      - id: len_ac_code_data_compressed
        type: u4
      - id: len_ac_code_data_uncompressed
        type: u4
      - id: ac_code_data_compressed
        size: len_ac_code_data_compressed
      - id: len_ac_data
        type: u4
      - id: ac_data
        size: len_ac_data
      - id: len_dc_data
        type: u4
      - id: dc_data
        size: len_dc_data
  animation_frame:
    seq:
      - id: header
        contents: [0x11, 0xC2, 0xB1, 0xA0]
      - id: size
        type: u4
      - id: num_block_start_positions
        type: u4
        if: size > 8
      - id: block_start_positions
        repeat: expr
        repeat-expr: num_block_start_positions
        type: u4
        if: size > 8
      - id: image_data
        type: image
        if: size > 8
  block_positions:
    seq:
      - id: header
        contents: [0x11, 0xC2, 0xB1, 0xA0]
      - id: size
        type: u4
  animation:
    seq:
      - id: header
        contents: [0x1, 0xC2, 0xB1, 0xA0]
      - id: size
        type: u4
      - id: name
        type: str
        encoding: UTF-8
        size: 0x20
      - id: num_frames
        type: u4
      - id: frames
        repeat: expr
        repeat-expr: num_frames
        type: animation_frame

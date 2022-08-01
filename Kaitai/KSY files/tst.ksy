meta:
  id: tst
  title: TST file
  file-extension: tst
  endian: le
  bit-endian: le
seq:
  - id: num_zones
    type: u4
  - id: zones
    type: zone
    repeat: expr
    repeat-expr: num_zones
types:
  zone:
    seq:
      - id: unkn_1
        type: u4
      - id: unkn_2
        type: u4
      - id: unkn_3
        type: u4
      - id: unkn_4
        type: u4

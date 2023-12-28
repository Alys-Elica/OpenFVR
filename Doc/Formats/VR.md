# VR files

Note: this format is not fully documented yet.

VR files contains images and animations used for static screens and 360 environments.

360 panoramic images contains data for the 6 faces of a cube (512x512 pixels each). Each face is sliced in 4 sub-faces (256x256 pixels each) that are then stacked top to bottom in the main 256x6144 pixel image.

## File structure (little-endian)

| Address | Size (bytes) | Description                   |
| ------- | ------------ | ----------------------------- |
| 0x00    | 4            | Header (AB 84 FA 12)          |
| 0x04    | 4            | File size                     |
| 0x08    | 4            | Type                          |
| 0x0c    | 4            | Image data size (N1)          |
| 0x10    | N1           | Image data (documented later) |

Base image data is followed by optional animation data:

| Address | Size (bytes) | Description                       |
| ------- | ------------ | --------------------------------- |
| 0x00    | 4            | Animation header (0xa0b1c201)     |
| 0x04    | 4            | Animation data size               |
| 0x08    | N2           | Animation data (documented later) |

## Basic informations

The type determines if the VR file is a static 640x480 image (type value == -0x5f4e3c00), a 360 projected 256x6144 image (type value == -0x5f4e3e00).

## Image data

Image data is not yet fully understood but DCT is used to encode the images.

| Address | Size (bytes) | Description                      |
| ------- | ------------ | -------------------------------- |
| 0x00    | 4            | Quality                          |
| 0x04    | 4            | Data size                        |
| 0x0c    | 4            | Image AC codes compressed size   |
| 0x10    | 4            | Image AC codes uncompressed size |
| 0x14    | N1           | Image AC codes compressed data   |
| 0xXX    | 4            | Image AC data size               |
| 0xXX    | N2           | Image AC data (can be empty)     |
| 0xXX    | 4            | Image DC data size               |
| 0xXX    | N3           | Image DC data                    |

### AC code compression

AC codes data is compressed using Huffman algorithm.

The compressed data begins with a compacted header that needs to be unpacked into a 256 element array storing normalized byte frequencies. To unpack the header :

- Read startOffset (1 byte)
- Read endOffset (1 byte)
- Read `endOffset - startOffset + 1` bytes and store them in the output array starting from position startOffset
- Repeat until startOffset value reads 0

Read frequencies are used to build an Huffman tree wich is used to uncompress the AC code data.

### Image reconstruction

Compression is similar to that of the 4XM video format. Here is the general algorithm:

- First, a 8x8 block is unpacked (in YCbCr format)
- The block is put in it right pixel order (zigzag)
- Each YCbCr component of the block is multiplied with a luma dequant table (for Y) and a chroma dequant table (for Cb and Cr). Each of these table is precalculated usimg the `quality` of the image
- Inverse DCT is applied on each 3 component, and clamped to the [-128, 127] range
- The block is then transformed to RGB
- Rinse and repeat for each 8x8 block, in left to right then top to bottom order

> TODO: document block unpacking

> TODO: document block transformation to RGB

## Optional animation data

There can be one or more animation present in the VR file (torches, fires, objects that can be picked, ...). Each animation has a name and a certain number of frames (using the same DCT compression algorihm as the image).

The structure is as follows:

| Address | Size (bytes) | Description                   |
| ------- | ------------ | ----------------------------- |
| 0x00    | 20           | Name ('\0' terminated string) |
| 0x14    | 4            | Frame count (`frameCount`)    |

After that follows `frameCount` frames structured as follows :

| Address | Size (bytes)    | Description                                  |
| ------- | --------------- | -------------------------------------------- |
| 0x00    | 4               | Header (0xa0b1c211)                          |
| 0x04    | 4               | Size (if 8, empty frame -> go to next frame) |
| 0x08    | 4               | Block count (`blockCount`)                   |
| 0x0c    | 4 \* blockCount | List of each block `pixelOffset`             |
| 0xXX    | 4               | Image data (same as previously documented)   |

The image data uses the same DCT compression as above. It stores each `blockCount` 8x8 block that compose the frame. Each block has a corresponding `pixelOffset` that defines where it goes on the image.

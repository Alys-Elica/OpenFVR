# VR files

Note: this format is not fully documented yet.

VR files contains images and animations used for static screens and 360 environments.

## File structure (little-endian)

| Address | Size (bytes) | Description                   |
| ------- | ------------ | ----------------------------- |
| 0x00    | 4            | Header (AB 84 FA 12)          |
| 0x04    | 4            | File size                     |
| 0x08    | 4            | Type                          |
| 0x0c    | 4            | Image data size (N1)          |
| 0x10    | N1           | Image data (documented later) |

After that can follow optional data parts (0 or more). Each additional part consists of a 4 byte header, a 4 byte size N followed by N bytes representing the additional part data (documented later)

| Address | Size (bytes) | Description                                          |
| ------- | ------------ | ---------------------------------------------------- |
| 0x00    | 4            | Part header (only seen a value of 0xa0b1c201 so far) |
| 0x04    | 4            | Part size                                            |
| 0x08    | N2           | Part 2 data                                          |

## Basic informations

The type determines if the VR file is a static 640x480 image (type value == -0x5f4e3c00), a 360 projected 256x6144 image (type value == -0x5f4e3e00). A third type may exist, but is not yet documented

## Image data

Image data is not yet fully understood but DCT might be used to encode the images.

| Address | Size (bytes) | Description                                              |
| ------- | ------------ | -------------------------------------------------------- |
| 0x00    | 4            | Quality ? (only seen a value of 100 so far)              |
| 0x04    | 4            | Data size                                                |
| 0x0c    | 4            | Image part 1 size                                        |
| 0x10    | 4            | Image part 1 uncompressed size ?                         |
| 0x14    | N1           | Image part 1                                             |
| 0xXX    | 4            | Image part 2 size                                        |
| 0xXX    | N2           | Image part 2 (can be empty)                              |
| 0xXX    | 4            | Image part 3 size                                        |
| 0xXX    | N3           | Image part 3 (1/8th of the image pixel count \* 3 bytes) |

## Additional part data

These parts are (again) not fully documented and are not always present.

Each part seems to contain a certain number of images (formated like in part 1) with additional information. They are supposed to be one of the following :

-   Animation: like a torch flame
-   Image patch: maybe objects that disapear after being picked or alternate environment parts (open/closed interactive door for example)

| Address | Size (bytes) | Description                   |
| ------- | ------------ | ----------------------------- |
| 0x00    | 20           | Name ('\0' terminated string) |
| 0x14    | 4            | Frame count (N)               |

After that follows N parts structured as follows :

| Address | Size (bytes) | Description                                             |
| ------- | ------------ | ------------------------------------------------------- |
| 0x00    | 4            | Header (only seen a value of 0xa0b1c211 so far)         |
| 0x04    | 4            | Size (If its value is 8, this sub-part data stops here) |
| 0x08    | 4            | Sub-count (N)                                           |
| 0x0c    | 4            | N elements of 4 byte each (still unknown)               |
| 0xXX    | 4            | Image data (same as previously documented)              |

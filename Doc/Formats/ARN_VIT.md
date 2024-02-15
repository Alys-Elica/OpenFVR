# ARN and VIT files

Note: these formats are not fully documented yet. Some assumptions are made that can be incomplete/incorrect.

These files are used as image archives and work in tandem: VIT files act as header and ARN files store raw file content.

## File structure (little-endian)

### VIT

Any VIT file contains the following:

| Address | Size (bytes) | Description           |
| ------- | ------------ | --------------------- |
| 0x00    | 4            | File count (N)        |
| 0x04    | 4            | Unknown               |
| 0x08    | N\*60        | N 60 byte file blocks |

Each file block stores information for one file of the archive and is structured as follows:

| Address | Size (bytes) | Description                                |
| ------- | ------------ | ------------------------------------------ |
| 0x00    | 32           | File name ('\0' terminated)                |
| 0x20    | 4            | Unknown (only 0 seen so far)               |
| 0x24    | 4            | Unknown (only 0 seen so far)               |
| 0x28    | 4            | Width                                      |
| 0x2C    | 4            | Height                                     |
| 0x30    | 4            | Unknown (only 1 seen so far)               |
| 0x34    | 4            | File size (N)                              |
| 0x38    | 4            | Unknown (values of 1, 2 and 4 seen so far) |

### ARN

ARN files are much more simple. They are just composed of the archived files content concatenated together in order of their appearance in the VIT file. Simply read the N bytes (archived file size) read in the VIT file.

Images are formated as RGB565 (16 bits per pixel) and are not compressed.

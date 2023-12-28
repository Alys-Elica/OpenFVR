# ARN and VIT files

Note: these formats are not fully documented yet. Some assumptions are made that can be incomplete/incorrect.

These files are used as (only image ?) archives and work in tandem: VIT files ("header" files) stores file informations and ARN files ("data" files) stores files content.

## File structure (little-endian)

### VIT

Any VIT file contains a 4 byte N file count, 4 bytes of unknown value followed by N 60 byte blocks, each storing information for one file of the archive. Each block has is structured as follows:

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

One particular thing to note is the need to manage BMP row pixel padding in case the image width isn't a multiple of 4. In that case add the 2 byte values 0x00 and 0xFF at the end of each line data.

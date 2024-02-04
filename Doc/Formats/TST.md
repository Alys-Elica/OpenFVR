# TST files

TST files contains coordinates for clickable VR zones (doors, objects, etc.) in the corresponding VR file.

| Address | Size (bytes) | Description               |
| ------- | ------------ | ------------------------- |
| 0x00    | 4            | Zone count (N)            |
| 0x08    | N            | Zone coordinates          |


Per zone entry is stored as follows:

| Offset | Size (bytes) | Description   |
| ------ | ------------ | ------------- |
| 0x00   | 4 (float)    | x1 coordinate |
| 0x04   | 4 (float)    | x2 coordinate |
| 0x08   | 4 (float)    | y1 coordinate |
| 0x10   | 4 (float)    | y2 coordinate |

The four coordinates together form the interactive zone. One should not assume that the coordinates 
coorespond to any specific direction, i.e. x1 is not guaranteed to always be the left or right corner
or vice-versa. The pairs will, however, always be on the same axis.

For flat VR files, each coordinate is the pixel offset of the drawn VR image.
For panoramic VR files, the coordinates are stored as degrees.
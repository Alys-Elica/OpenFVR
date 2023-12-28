# PAK files

PAK files are just compressed archive.
Every file is composed as follows (little-endian except when specified) :

-   4 bytes 'PAKF' header
-   4 bytes file size

| Address | Size (bytes) | Description                         |
| ------- | ------------ | ----------------------------------- |
| 0x00    | 4            | 'PAKF' header                       |
| 0x04    | 4            | File size                           |
| 0x08    | x\*n bytes   | Set of files present in the archive |

Every file in the archive is stored as follows:

| Address (add global file offset) | Size (bytes)    | Description                                                                  |
| -------------------------------- | --------------- | ---------------------------------------------------------------------------- |
| 0x00                             | 16              | Null terminated file name                                                    |
| 0x10                             | 4               | Compression level or type (still unclear; only encountered value '3' so far) |
| 0x14                             | 4               | Compressed size (size of data to read in the archive)                        |
| 0x18                             | 4               | Uncompressed size                                                            |
| 0x1c                             | Compressed size | Compressed file data                                                         |

Data decompression makes use of the already decompressed data as it unfolds (LZ77 ?). It works as follows for compression level/type '3' :

-   Read 1 byte
    -   If the most significant bit (8th) is NOT SET, just read 'byte + 1' bytes as uncompressed data and add it to the output
    -   If the most significant bit (8th) is SET :
        -   The 6 less significant bits represents the data size N to read
        -   The second most significant bit (7th) determines the next offset O size to read (0 => 2 bytes (in big-endian order); 1 => 1 byte)
        -   Read the N bytes from the OUTPUT (not the compressed data) starting at O bytes from the end and add them directly (byte by byte) to the output

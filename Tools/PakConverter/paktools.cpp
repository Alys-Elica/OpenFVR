#include "paktools.h"

#include <bitset>
#include <iostream>

#include <fvr/file.h>

bool PakTools::unpack(const std::string &fileName, std::vector<PakSubFile> &uncompressedFiles)
{
    File file;
    file.setEndian(File::Endian::LittleEndian);

    if (!file.open(fileName, std::ios_base::in | std::ios_base::binary))
    {
        std::cerr << "Unable to open file " << fileName << std::endl;
        return false;
    }

    char header[5] = {0, 0, 0, 0, 0};
    file.read(header, 4);

    uint32_t fileSize;
    file >> fileSize;

    uncompressedFiles.clear();
    while (!file.atEnd())
    {
        char compFileName[16]; // Compressed file name
        file.read(compFileName, 16);

        uint32_t compressionLevel;
        file >> compressionLevel;

        uint32_t compressedSize; // Compressed file size
        file >> compressedSize;

        uint32_t uncompressedSize; // Compressed file originalSize (uncompressed)
        file >> uncompressedSize;

        std::vector<uint8_t> compressedData(compressedSize, 0x00);
        file.read(compressedData.data(), compressedSize);

        PakSubFile subFile;

        subFile.fileName = std::string(compFileName);

        switch (compressionLevel)
        {
        case 3:
            uncompressPakData3(compressedData, subFile.data);
            break;

        default:
            std::cerr << "Compression not yet known" << std::endl;
            break;
        }

        if (subFile.data.size() != uncompressedSize)
        {
            std::cerr << "Uncompressed size does not match" << std::endl;
            std::cerr << "    Expected: " << uncompressedSize << std::endl;
            std::cerr << "    Actual: " << subFile.data.size() << std::endl;
        }

        uncompressedFiles.push_back(subFile);
    }

    file.close();

    return true;
}

void PakTools::uncompressPakData3(const std::vector<uint8_t> &dataIn, std::vector<uint8_t> &dataOut)
{
    dataOut.clear();

    size_t idxIn = 0;
    while (idxIn < dataIn.size())
    {
        uint8_t byte = dataIn[idxIn++];

        if (std::bitset<8>(byte)[7])
        {
            /*
             * If most significant bit is set to 1
             *      => copy of N1 bytes from already uncompressed data starting at N2 bytes from the end of
             *         current outputed data
             *              N1 = byte & 0x3f;
             *              N2 = index (1 byte if second most significant bit is 1, 2 bytes if 0)
             */

            int size = (byte & 0x3f) + 1;
            int index;

            if (std::bitset<8>(byte)[6])
            {
                uint8_t tmpIndex = dataIn[idxIn++];
                index = dataOut.size() - (tmpIndex + 1);
            }
            else
            {
                uint16_t tmpIndex;

                // Stored in big endian
                tmpIndex = dataIn[idxIn++] << 8;
                tmpIndex |= dataIn[idxIn++];

                index = dataOut.size() - (tmpIndex + 1);
            }

            for (int i = 0; i < size; ++i)
            {
                dataOut.push_back(dataOut[index++]);
            }
        }
        else
        {
            for (int i = 0; i < byte + 1; ++i)
            {
                dataOut.push_back(dataIn[idxIn++]);
            }
        }
    }
}

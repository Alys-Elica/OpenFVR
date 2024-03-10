#include "fvr_pak.h"

#include <bitset>
#include <fstream>
#include <iostream>
#include <vector>

#include "fvr/file.h"

/* Private */
struct PakFile
{
    std::string fileName;
    uint32_t compressedSize;
    uint32_t uncompressedSize;
    uint32_t compressionLevel;

    std::vector<uint8_t> compressedData;
};

class FvrPak::FvrPakPrivate
{
    friend class FvrPak;

public:
    static void uncompressPakData3(const std::vector<uint8_t> &dataIn, std::vector<uint8_t> &dataOut);

private:
    File filePak;
    std::vector<PakFile> listFile;
};

void FvrPak::FvrPakPrivate::uncompressPakData3(const std::vector<uint8_t> &dataIn, std::vector<uint8_t> &dataOut)
{
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

/* Public */
FvrPak::FvrPak()
{
    d_ptr = new FvrPakPrivate;
}

FvrPak::~FvrPak()
{
    delete d_ptr;
}

bool FvrPak::open(const std::string &pakFileName)
{
    d_ptr->filePak.setEndian(std::endian::little);

    if (!d_ptr->filePak.open(pakFileName, std::ios_base::in | std::ios_base::binary))
    {
        std::cerr << "Unable to open file " << pakFileName << std::endl;
        return false;
    }

    char header[5] = {0, 0, 0, 0, 0};
    d_ptr->filePak.read(header, 4);

    uint32_t fileSize;
    d_ptr->filePak >> fileSize;

    d_ptr->listFile.clear();
    while (!d_ptr->filePak.atEnd())
    {
        PakFile subFile;

        char compFileName[16]; // Compressed file name
        d_ptr->filePak.read(compFileName, 16);
        subFile.fileName = std::string(compFileName);

        d_ptr->filePak >> subFile.compressionLevel;
        d_ptr->filePak >> subFile.compressedSize;
        d_ptr->filePak >> subFile.uncompressedSize;

        subFile.compressedData.resize(subFile.compressedSize, 0x00);
        d_ptr->filePak.read(subFile.compressedData.data(), subFile.compressedSize);

        d_ptr->listFile.push_back(subFile);
    }

    return true;
}

void FvrPak::close()
{
    d_ptr->filePak.close();
}

bool FvrPak::isOpen() const
{
    return d_ptr->filePak.isOpen();
}

int FvrPak::fileCount() const
{
    return d_ptr->listFile.size();
}

std::string FvrPak::fileName(int index) const
{
    return d_ptr->listFile[index].fileName;
}

std::vector<uint8_t> FvrPak::fileData(int index) const
{
    if (index < 0 || index >= d_ptr->listFile.size())
    {
        std::cerr << "Index out of range" << std::endl;
        return std::vector<uint8_t>();
    }

    if (!isOpen())
    {
        std::cerr << "File not open" << std::endl;
        return std::vector<uint8_t>();
    }

    PakFile &subFile = d_ptr->listFile[index];

    std::vector<uint8_t> uncompressedData;
    uncompressedData.reserve(subFile.uncompressedSize);
    switch (subFile.compressionLevel)
    {
    case 3:
        FvrPakPrivate::uncompressPakData3(subFile.compressedData, uncompressedData);
        break;

    default:
        std::cerr << "Compression not yet known" << std::endl;
        break;
    }

    if (uncompressedData.size() != subFile.uncompressedSize)
    {
        std::cerr << "Uncompressed size does not match" << std::endl;
        std::cerr << "    Expected: " << subFile.uncompressedSize << std::endl;
        std::cerr << "    Actual: " << uncompressedData.size() << std::endl;

        return std::vector<uint8_t>();
    }

    return uncompressedData;
}

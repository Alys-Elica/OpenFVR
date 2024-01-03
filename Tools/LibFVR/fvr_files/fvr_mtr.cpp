#include "fvr_mtr.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

/* Private */
class FvrMtrPrivate
{
    friend class FvrMtr;

public:
    FvrMtrPrivate();
    ~FvrMtrPrivate();

    static void rleDecompress(const std::vector<uint8_t> &data, std::vector<uint8_t> &dest);

    bool checkMagic();
    bool checkType();
    bool readSize();
    bool checkFileSize();
    bool readData();

private:
    std::ifstream fileMtr;
    std::string type;
    uint32_t width = 0;
    uint32_t height = 0;
    uint64_t size = 0;
    uint8_t palette[0x300]; // 256 RGB 24 bits colors
    Image image;
};

FvrMtrPrivate::FvrMtrPrivate()
{
    type.resize(2);
}

FvrMtrPrivate::~FvrMtrPrivate()
{
}

void FvrMtrPrivate::rleDecompress(const std::vector<uint8_t> &data, std::vector<uint8_t> &dest)
{
    size_t i = 0;
    while (i < data.size())
    {
        uint8_t count = data[i++];
        if (count < 128)
        {
            count++;
            uint8_t value = data[i++];
            for (int k = 0; k < count; k++)
            {
                dest.push_back(value);
            }
        }
        else
        {
            count = count - 127;
            for (int k = 0; k < count; k++)
            {
                dest.push_back(data[i++]);
            }
        }
    }
}

bool FvrMtrPrivate::checkMagic()
{
    fileMtr.seekg(0);
    std::string magic(5, '\0');
    fileMtr.read(magic.data(), 5);
    return magic == "MDRAW";
}

bool FvrMtrPrivate::checkType()
{
    fileMtr.seekg(0x15);
    fileMtr.read(type.data(), 2);
    return type == "NC" || type == "FC";
}

bool FvrMtrPrivate::readSize()
{
    // Read width
    fileMtr.seekg(0x06);
    std::string strWidth(4, '\0');
    fileMtr.read(strWidth.data(), 4);

    std::istringstream(strWidth) >> std::hex >> width;
    ++width; // Width is 1 less than actual width

    // Read height
    fileMtr.seekg(0x0B);
    std::string strHeight(4, '\0');
    fileMtr.read(strHeight.data(), 4);

    std::istringstream(strHeight) >> std::hex >> height;
    ++height; // Height is 1 less than actual height

    return width > 0 && height > 0 && width <= 1023 && height <= 1023;
}

bool FvrMtrPrivate::checkFileSize()
{
    fileMtr.seekg(0x18);
    std::string strSize(8, '\0');
    fileMtr.read(strSize.data(), 8);

    std::istringstream(strSize) >> std::hex >> size;

    fileMtr.seekg(0, std::ios_base::end);

    return size == fileMtr.tellg();
}

bool FvrMtrPrivate::readData()
{
    fileMtr.seekg(0x10);
    std::string tmp(4, '\0');
    fileMtr.read(tmp.data(), 4);
    if (tmp == "00FF")
    {
        if (type == "NC")
        { // MST NC 00FF
            // TODO: implement
            std::cerr << "NOT IMPLEMENTED" << std::endl;
            return false;
        }
        else if (type == "FC")
        { // MST FC 00FF
            fileMtr.seekg(-8, std::ios_base::end);
            std::streampos end = fileMtr.tellg();

            // Size minus header (0x32 bytes), palette (0x300 bytes) and reserved end string (8 bytes)
            uint64_t dataSize = size - 0x32 - 0x300 - 8;
            std::vector<uint8_t> source(dataSize);
            fileMtr.seekg(0x332);
            fileMtr.read((char *)source.data(), dataSize);

            std::vector<uint8_t> decompressed;
            decompressed.reserve(width * height);

            rleDecompress(source, decompressed);

            if (decompressed.size() != width * height)
            {
                std::cerr << "Invalid decompressed size" << std::endl;
                return false;
            }

            if (!image.resize(width, height))
            {
                std::cerr << "Failed to resize out image" << std::endl;
                return false;
            }

            for (size_t i = 0; i < decompressed.size(); i++)
            {
                image.setPixel(
                    i % width, i / width,
                    {palette[decompressed[i] * 3 + 0],
                     palette[decompressed[i] * 3 + 1],
                     palette[decompressed[i] * 3 + 2], 255});
            }

            return true;
        }
    }
    else
    {
        if (tmp[2] == 'P')
        {
            if (type == "NC")
            { // MST NC 00xP
                // TODO: implement
                std::cerr << "NOT IMPLEMENTED" << std::endl;
                return false;
            }
            else if (type == "FC")
            { // MST FC 00xP
                // TODO: implement
                std::cerr << "NOT IMPLEMENTED" << std::endl;
                return false;
            }
        }
        else
        {
            std::cerr << "Invalid type" << std::endl;
            return false;
        }
    }
}

/* Public */
FvrMtr::FvrMtr()
{
    d_ptr = new FvrMtrPrivate();
}

FvrMtr::~FvrMtr()
{
    delete d_ptr;
}

bool FvrMtr::open(const std::string &mtrFileName)
{
    d_ptr->fileMtr.open(mtrFileName, std::ios_base::in | std::ios_base::binary);

    if (!d_ptr->fileMtr.is_open())
    {
        std::cerr << "Failed to open " << mtrFileName << std::endl;
        return false;
    }

    // Check magic
    if (!d_ptr->checkMagic())
    {
        std::cerr << "Invalid magic" << std::endl;
        return false;
    }

    // Check type
    if (!d_ptr->checkType())
    {
        std::cerr << "Invalid type" << std::endl;
        return false;
    }

    // Read size
    if (!d_ptr->readSize())
    {
        std::cerr << "Invalid size" << std::endl;
        return false;
    }

    // Read size
    if (!d_ptr->checkFileSize())
    {
        std::cerr << "Invalid file size" << std::endl;
        return false;
    }

    // Read palette
    d_ptr->fileMtr.seekg(0x32);
    d_ptr->fileMtr.read(reinterpret_cast<char *>(d_ptr->palette), 0x300);

    // Read data
    if (!d_ptr->readData())
    {
        std::cerr << "Invalid data" << std::endl;
        return false;
    }
}

void FvrMtr::close()
{
    d_ptr->fileMtr.close();
}

bool FvrMtr::isOpen() const
{
    return d_ptr->fileMtr.is_open();
}

uint32_t FvrMtr::width() const
{
    return d_ptr->width;
}

uint32_t FvrMtr::height() const
{
    return d_ptr->height;
}

Image const &FvrMtr::image() const
{
    return d_ptr->image;
}

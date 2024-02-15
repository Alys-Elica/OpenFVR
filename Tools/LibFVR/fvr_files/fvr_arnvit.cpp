#include "fvr_arnvit.h"

#include <iostream>

#include "fvr/file.h"

/* Private */
class FvrArnVitPrivate
{
    friend class FvrArnVit;

private:
    File fileVit;
    File fileArn;

    std::vector<FvrArnVit::ArnVitFile> fileList;
};

/* Public */
FvrArnVit::FvrArnVit()
{
    d_ptr = new FvrArnVitPrivate();
}

FvrArnVit::~FvrArnVit()
{
    delete d_ptr;
}

bool FvrArnVit::open(const std::string &vitFileName, const std::string &arnFileName)
{
    if (!d_ptr->fileVit.open(vitFileName, std::ios::binary | std::ios::in))
    {
        std::cerr << "Unable to open VIT file " << vitFileName << std::endl;
        return false;
    }

    if (!d_ptr->fileArn.open(arnFileName, std::ios::binary | std::ios::in))
    {
        std::cerr << "Unable to open ARN file " << arnFileName << std::endl;
        return false;
    }

    d_ptr->fileList.clear();

    // Parse VIT header file
    d_ptr->fileVit.setEndian(File::Endian::LittleEndian);

    uint32_t fileCount;
    d_ptr->fileVit >> fileCount;

    uint32_t unkn;
    d_ptr->fileVit >> unkn;

    uint32_t offset = 0;
    for (uint32_t i = 0; i < fileCount; i++)
    {
        ArnVitFile file;

        char fileName[32];
        d_ptr->fileVit.read(fileName, 32);
        file.fileName = std::string(fileName);

        d_ptr->fileVit >> file.unkn1;
        d_ptr->fileVit >> file.unkn2;
        d_ptr->fileVit >> file.width;
        d_ptr->fileVit >> file.height;
        d_ptr->fileVit >> file.unkn3;
        d_ptr->fileVit >> file.fileSize;
        d_ptr->fileVit >> file.unkn4;

        file.offset = offset;

        offset += file.fileSize;

        d_ptr->fileList.push_back(file);
    }

    return true;
}

void FvrArnVit::close()
{
    d_ptr->fileVit.close();
    d_ptr->fileArn.close();
}

bool FvrArnVit::isOpen() const
{
    return d_ptr->fileVit.isOpen() && d_ptr->fileArn.isOpen();
}

int FvrArnVit::fileCount() const
{
    return d_ptr->fileList.size();
}

FvrArnVit::ArnVitFile FvrArnVit::getFile(const int index) const
{
    ArnVitFile file = d_ptr->fileList[index];

    d_ptr->fileArn.seek(file.offset);
    file.data.resize(file.fileSize);
    d_ptr->fileArn.read(file.data.data(), file.fileSize);

    return file;
}

bool FvrArnVit::writeToBmp(const int index, const std::string &outputDirectory) const
{
    ArnVitFile file = getFile(index);
    if (file.data.empty())
    {
        std::cerr << "Unable to read file data" << std::endl;
        return false;
    }

    std::string bmpFile = outputDirectory + file.fileName;
    File fileBmp;
    fileBmp.setEndian(File::Endian::LittleEndian);
    if (!fileBmp.open(bmpFile, std::ios::binary | std::ios::out))
    {
        std::cerr << "Unable to open BMP file " << bmpFile << std::endl;
        return false;
    }

    // BMP header
    fileBmp << uint16_t(0x4D42); // BM
    fileBmp << uint32_t(0);      // File size (will be updated later)
    fileBmp << uint16_t(0);      // Reserved
    fileBmp << uint16_t(0);      // Reserved
    fileBmp << uint32_t(54);     // Offset to image data

    // DIB header
    fileBmp << uint32_t(40);           // DIB header size
    fileBmp << uint32_t(file.width);   // Width
    fileBmp << uint32_t(-file.height); // Height
    fileBmp << uint16_t(1);            // Planes
    fileBmp << uint16_t(16);           // Bits per pixel
    fileBmp << uint32_t(0);            // Compression
    fileBmp << uint32_t(0);            // Image size (ignored for uncompressed images)
    fileBmp << uint32_t(0);            // X pixels per meter
    fileBmp << uint32_t(0);            // Y pixels per meter
    fileBmp << uint32_t(0);            // Colors in color table
    fileBmp << uint32_t(0);            // Important color count

    // Image data
    bool addPadding = (file.width * 2) % 4 != 0;
    for (uint32_t y = 0; y < file.height; y++)
    {
        // fileBmp.write(data.data() + y * file.width * 2, file.width * 2);
        for (uint32_t x = 0; x < file.width; x++)
        {
            int idx = (y * file.width + x) * 2;
            uint16_t pixel = (file.data[idx + 1] << 8) | file.data[idx];

            // RGB565 to RGB555
            pixel = ((pixel & 0b1111'1000'0000'0000) >> 1 |
                     (pixel & 0b0000'0111'1100'0000) >> 1 |
                     (pixel & 0b0000'0000'0001'1111));

            fileBmp << pixel;
        }

        if (addPadding)
        {
            fileBmp << uint8_t(0x00);
            fileBmp << uint8_t(0xFF);
        }
    }

    // Write file size
    uint32_t fileSize = fileBmp.tell();
    fileBmp.seek(2);
    fileBmp << uint32_t(fileSize);

    fileBmp.close();

    return true;
}

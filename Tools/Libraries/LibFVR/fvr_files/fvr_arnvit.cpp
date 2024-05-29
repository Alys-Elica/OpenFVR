#include "fvr_arnvit.h"

#include <fstream>
#include <iostream>
#include <map>

#include "fvr/datastream.h"

/* Private */
class FvrArnVit::FvrArnVitPrivate {
    friend class FvrArnVit;

private:
    std::ifstream fileVit;
    std::ifstream fileArn;

    std::vector<FvrArnVit::ArnVitFile> fileList;
    std::map<std::string, int> fileNameMap;
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

bool FvrArnVit::open(const std::string& vitFileName, const std::string& arnFileName)
{
    d_ptr->fileVit.open(vitFileName, std::ios::binary | std::ios::in);
    if (!d_ptr->fileVit.is_open()) {
        std::cerr << "Unable to open VIT file " << vitFileName << std::endl;
        return false;
    }

    d_ptr->fileArn.open(arnFileName, std::ios::binary | std::ios::in);
    if (!d_ptr->fileArn.is_open()) {
        std::cerr << "Unable to open ARN file " << arnFileName << std::endl;
        return false;
    }

    d_ptr->fileList.clear();

    // Parse VIT header file
    DataStream ds(&d_ptr->fileVit);
    ds.setEndian(std::endian::little);

    uint32_t fileCount;
    ds >> fileCount;

    uint32_t unkn;
    ds >> unkn;

    uint32_t offset = 0;
    for (uint32_t i = 0; i < fileCount; i++) {
        ArnVitFile file;

        char fileName[32];
        ds.read(32, (uint8_t*)fileName);
        file.fileName = std::string(fileName);

        ds >> file.unkn1;
        ds >> file.unkn2;
        ds >> file.width;
        ds >> file.height;
        ds >> file.unkn3;
        ds >> file.fileSize;
        ds >> file.unkn4;

        file.offset = offset;

        offset += file.fileSize;

        d_ptr->fileList.push_back(file);

        d_ptr->fileNameMap[file.fileName] = i;
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
    return d_ptr->fileVit.is_open() && d_ptr->fileArn.is_open();
}

int FvrArnVit::fileCount() const
{
    return d_ptr->fileList.size();
}

FvrArnVit::ArnVitFile FvrArnVit::getFile(const int index) const
{
    ArnVitFile file = d_ptr->fileList[index];

    d_ptr->fileArn.seekg(file.offset);
    file.data.resize(file.fileSize);
    d_ptr->fileArn.read((char*)file.data.data(), file.fileSize);

    return file;
}

FvrArnVit::ArnVitFile FvrArnVit::getFile(const std::string& name) const
{
    auto it = d_ptr->fileNameMap.find(name);
    if (it == d_ptr->fileNameMap.end()) {
        return {};
    }

    return getFile(it->second);
}

bool FvrArnVit::writeToBmp(const int index, const std::string& outputDirectory) const
{
    ArnVitFile file = getFile(index);
    if (file.data.empty()) {
        std::cerr << "Unable to read file data" << std::endl;
        return false;
    }

    std::string bmpFile = outputDirectory + file.fileName;
    std::ofstream fileBmp(bmpFile, std::ios::binary | std::ios::out);
    if (!fileBmp.is_open()) {
        std::cerr << "Unable to open BMP file " << bmpFile << std::endl;
        return false;
    }

    DataStream ds(&fileBmp);
    ds.setEndian(std::endian::little);

    // BMP header
    ds << uint16_t(0x4D42); // BM
    ds << uint32_t(0); // File size (will be updated later)
    ds << uint16_t(0); // Reserved
    ds << uint16_t(0); // Reserved
    ds << uint32_t(54); // Offset to image data

    // DIB header
    ds << uint32_t(40); // DIB header size
    ds << uint32_t(file.width); // Width
    ds << uint32_t(-file.height); // Height
    ds << uint16_t(1); // Planes
    ds << uint16_t(16); // Bits per pixel
    ds << uint32_t(0); // Compression
    ds << uint32_t(0); // Image size (ignored for uncompressed images)
    ds << uint32_t(0); // X pixels per meter
    ds << uint32_t(0); // Y pixels per meter
    ds << uint32_t(0); // Colors in color table
    ds << uint32_t(0); // Important color count

    size_t fileSize = 54;

    // Image data
    bool addPadding = (file.width * 2) % 4 != 0;
    for (uint32_t y = 0; y < file.height; y++) {
        // fileBmp.write(data.data() + y * file.width * 2, file.width * 2);
        for (uint32_t x = 0; x < file.width; x++) {
            int idx = (y * file.width + x) * 2;
            uint16_t pixel = (file.data[idx + 1] << 8) | file.data[idx];

            // RGB565 to RGB555
            pixel = ((pixel & 0b1111'1000'0000'0000) >> 1 | (pixel & 0b0000'0111'1100'0000) >> 1 | (pixel & 0b0000'0000'0001'1111));

            ds << pixel;

            fileSize += 2;
        }

        if (addPadding) {
            ds << uint8_t(0x00);
            ds << uint8_t(0xFF);

            fileSize += 2;
        }
    }

    // Write file size
    fileBmp.seekp(2);
    ds << uint32_t(fileSize);

    fileBmp.close();

    return true;
}

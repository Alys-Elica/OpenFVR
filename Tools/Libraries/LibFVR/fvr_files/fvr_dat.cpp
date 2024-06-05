#include "fvr_dat.h"

#include <fstream>
#include <iostream>
#include <vector>

#include "fvr/datastream.h"

/* Private */
struct DatFile {
    std::string name;
    uint32_t size;
    uint32_t offset;
};

class FvrDat::FvrDatPrivate {
    friend class FvrDat;

private:
    std::fstream fileDat;
    std::vector<DatFile> listFile;
    size_t dataOffset = -1;
};

/* Public */
FvrDat::FvrDat()
{
    d_ptr = new FvrDatPrivate();
}

FvrDat::~FvrDat()
{
    delete d_ptr;
}

bool FvrDat::open(const std::string& datFileName)
{
    d_ptr->fileDat.open(datFileName, std::ios_base::in | std::ios_base::binary);
    if (!d_ptr->fileDat.is_open()) {
        std::cerr << "Failed to open " << datFileName << std::endl;
        return false;
    }

    d_ptr->listFile.clear();

    DataStream ds(&d_ptr->fileDat);
    ds.setEndian(std::endian::little);

    // Check magic
    d_ptr->fileDat.seekg(0);
    std::string magic(4, '\0');
    ds.read(4, (uint8_t*)magic.data());
    if (magic != "BIGF") {
        std::cerr << "Invalid magic" << std::endl;
        return false;
    }

    // Read file index
    d_ptr->fileDat.seekg(0x50);
    uint8_t current;
    ds.read(1, &current);
    do {
        DatFile file;
        while (current != '\0') {
            file.name += current;
            ds.read(1, &current);
        }

        ds >> file.size;
        ds >> file.offset;

        d_ptr->listFile.push_back(file);

        ds.read(1, &current);
    } while (current != '\0');

    d_ptr->dataOffset = d_ptr->fileDat.tellg();

    return true;
}

void FvrDat::close()
{
    d_ptr->fileDat.close();
}

bool FvrDat::isOpen() const
{
    return d_ptr->fileDat.is_open();
}

int FvrDat::fileCount() const
{
    return d_ptr->listFile.size();
}

std::string FvrDat::fileName(const int& index) const
{
    if (index < 0 || index >= d_ptr->listFile.size()) {
        return "";
    }

    return d_ptr->listFile[index].name;
}

uint32_t FvrDat::fileSize(const int& index) const
{
    if (index < 0 || index >= d_ptr->listFile.size()) {
        return -1;
    }

    return d_ptr->listFile[index].size;
}

std::vector<uint8_t> FvrDat::fileData(const int& index) const
{
    if (index < 0 || index >= d_ptr->listFile.size()) {
        return std::vector<uint8_t>();
    }

    d_ptr->fileDat.seekg(d_ptr->listFile[index].offset + d_ptr->dataOffset);
    std::vector<uint8_t> data(d_ptr->listFile[index].size);
    d_ptr->fileDat.read((char*)data.data(), d_ptr->listFile[index].size);

    return data;
}

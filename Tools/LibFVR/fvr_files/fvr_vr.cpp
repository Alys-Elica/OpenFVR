#include "fvr_vr.h"

#include <iostream>

#include "fvr/file.h"
#include "internal/dct.h"

/* Private */
class FvrVrPrivate
{
    friend class FvrVr;

public:
    bool getRgb565Data(std::vector<uint8_t> &rgb565Data);
    bool rgb565DataToCubemap(const std::vector<uint8_t> &rgb565Data, std::vector<uint8_t> &cubemapRgb565Data);

private:
    File fileVr;
    FvrVr::Type type;
};

bool FvrVrPrivate::getRgb565Data(std::vector<uint8_t> &rgb565Data)
{
    if (!fileVr.isOpen())
    {
        std::cerr << "File is not open" << std::endl;
        return false;
    }

    Dct dct;
    if (!dct.isValid())
    {
        std::cerr << "DCT is not valid" << std::endl;
        return false;
    }

    fileVr.seek(12);

    uint32_t imageSize;
    fileVr >> imageSize;

    uint32_t quality;
    fileVr >> quality;

    uint32_t dataSize;
    fileVr >> dataSize; // File size - 16

    std::vector<uint8_t> rawData(dataSize);
    fileVr.read((char *)rawData.data(), dataSize);

    // Unpack image
    bool ret = false;
    if (type == FvrVr::Type::VR_STATIC_VR)
    {
        ret = dct.unpackVr(rawData, quality, rgb565Data);
    }
    else if (type == FvrVr::Type::VR_STATIC_PIC)
    {
        ret = dct.unpackPicture(rawData, quality, rgb565Data);
    }

    if (!ret)
    {
        std::cerr << "Failed to unpack image" << std::endl;
        return false;
    }

    return true;
}

bool FvrVrPrivate::rgb565DataToCubemap(const std::vector<uint8_t> &rgb565Data, std::vector<uint8_t> &cubemapRgb565Data)
{
    if (rgb565Data.size() != 256 * 6144 * 2)
    {
        std::cerr << "Invalid RGB565 data size" << std::endl;
        return false;
    }

    cubemapRgb565Data.resize(2048 * 1536 * 2);
    size_t faceOffsets[6] = {
        2048u * 512u * 2u * 2u + 512u * 2u,
        2048u * 512u * 2u,
        512u * 2u,
        2048u * 512u * 2u + 512u * 2u * 2u,
        2048u * 512u * 2u + 512u * 2u,
        2048u * 512u * 2u + 512u * 2u * 3u,
    };

    for (int faceIdx = 0; faceIdx < 6; ++faceIdx)
    {
        uint32_t faceOffset = faceOffsets[faceIdx];

        for (int lineIdx = 0; lineIdx < 256; ++lineIdx)
        {
            std::memcpy(
                cubemapRgb565Data.data() + faceOffset + lineIdx * 2048 * 2,
                rgb565Data.data() + faceIdx * 512 * 512 * 2 + lineIdx * 256 * 2,
                256 * 2);
            std::memcpy(
                cubemapRgb565Data.data() + faceOffset + 256 * 2 + lineIdx * 2048 * 2,
                rgb565Data.data() + faceIdx * 512 * 512 * 2 + 256 * 256 * 2 + lineIdx * 256 * 2,
                256 * 2);
            std::memcpy(
                cubemapRgb565Data.data() + faceOffset + 2048 * 256 * 2 + lineIdx * 2048 * 2,
                rgb565Data.data() + faceIdx * 512 * 512 * 2 + 3 * 256 * 256 * 2 + lineIdx * 256 * 2,
                256 * 2);
            std::memcpy(
                cubemapRgb565Data.data() + faceOffset + 256 * 2 + 2048 * 256 * 2 + lineIdx * 2048 * 2,
                rgb565Data.data() + faceIdx * 512 * 512 * 2 + 2 * 256 * 256 * 2 + lineIdx * 256 * 2,
                256 * 2);
        }
    }

    return true;
}

/* Public */
FvrVr::FvrVr()
{
    d_ptr = new FvrVrPrivate;

    d_ptr->fileVr.setEndian(File::Endian::LittleEndian);
    d_ptr->type = FvrVr::Type::VR_UNKNOWN;
}

FvrVr::~FvrVr()
{
    delete d_ptr;
}

bool FvrVr::open(const std::string &vrFileName)
{
    if (!d_ptr->fileVr.open(vrFileName, std::ios::binary | std::ios::in))
    {
        std::cerr << "Failed to open file " << vrFileName << std::endl;
        return false;
    }

    int32_t header;
    int32_t fileSize;
    int32_t type;

    d_ptr->fileVr >> header;
    d_ptr->fileVr >> fileSize;
    d_ptr->fileVr >> type;

    if (type == -0x5f4e3e00)
    {
        d_ptr->type = Type::VR_STATIC_VR;
    }
    else if (type == -0x5f4e3c00)
    {
        d_ptr->type = Type::VR_STATIC_PIC;
    }
    else
    {
        d_ptr->type = Type::VR_UNKNOWN;
    }
}

void FvrVr::close()
{
    d_ptr->fileVr.close();
}

bool FvrVr::isOpen() const
{
    return d_ptr->fileVr.isOpen();
}

int FvrVr::getWidth() const
{
    switch (d_ptr->type)
    {
    case FvrVr::Type::VR_STATIC_VR:
        return 256;
    case FvrVr::Type::VR_STATIC_PIC:
        return 640;
    default:
        return 0;
    }
}

int FvrVr::getHeight() const
{
    switch (d_ptr->type)
    {
    case FvrVr::Type::VR_STATIC_VR:
        return 6144;
    case FvrVr::Type::VR_STATIC_PIC:
        return 480;
    default:
        return 0;
    }
}

FvrVr::Type FvrVr::getType() const
{
    return d_ptr->type;
}

bool FvrVr::getImage(Image &image) const
{
    std::vector<uint8_t> rgb565Data;
    if (!d_ptr->getRgb565Data(rgb565Data))
    {
        std::cerr << "Failed to get RGB565 data" << std::endl;
        return false;
    }

    if (!image.fromRgb565(rgb565Data, getWidth(), getHeight()))
    {
        std::cerr << "Failed to convert image" << std::endl;
        return false;
    }

    return true;
}

bool FvrVr::getImageCubemap(Image &image) const
{
    std::vector<uint8_t> rgb565Data;
    if (!d_ptr->getRgb565Data(rgb565Data))
    {
        std::cerr << "Failed to get RGB565 data" << std::endl;
        return false;
    }

    std::vector<uint8_t> cubemapRgb565Data;
    if (!d_ptr->rgb565DataToCubemap(rgb565Data, cubemapRgb565Data))
    {
        std::cerr << "Failed to convert RGB565 data to cubemap" << std::endl;
        return false;
    }

    if (!image.fromRgb565(cubemapRgb565Data, 2048, 1536))
    {
        std::cerr << "Failed to convert image" << std::endl;
        return false;
    }

    return true;
}

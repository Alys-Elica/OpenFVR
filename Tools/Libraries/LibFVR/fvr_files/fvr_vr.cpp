#include "fvr_vr.h"

#include <algorithm>
#include <cstring>
#include <iostream>
#include <map>

#include "fvr/datastream.h"
#include "internal/dct.h"

/* Private */
class FvrVr::FvrVrPrivate {
    friend class FvrVr;

public:
    bool rgb565DataToCubemap(const std::vector<uint8_t>& rgb565Data, std::vector<uint8_t>& cubemapRgb565Data);

private:
    std::fstream m_fileVr;
    FvrVr::Type m_type;

    std::vector<uint8_t> m_rgb565Data;
    std::map<std::string, Animation> m_animationList;
};

bool FvrVr::FvrVrPrivate::rgb565DataToCubemap(const std::vector<uint8_t>& rgb565Data, std::vector<uint8_t>& cubemapRgb565Data)
{
    if (rgb565Data.size() != 256 * 6144 * 2) {
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

    for (int faceIdx = 0; faceIdx < 6; ++faceIdx) {
        uint32_t faceOffset = faceOffsets[faceIdx];

        for (int lineIdx = 0; lineIdx < 256; ++lineIdx) {
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

    d_ptr->m_type = FvrVr::Type::VR_UNKNOWN;
}

FvrVr::~FvrVr()
{
    delete d_ptr;
}

bool FvrVr::open(const std::string& vrFileName)
{
    d_ptr->m_fileVr.open(vrFileName, std::ios::binary | std::ios::in);
    if (!d_ptr->m_fileVr.is_open()) {
        // std::cerr << "Failed to open file " << vrFileName << std::endl;
        return false;
    }

    DataStream ds(&d_ptr->m_fileVr);
    ds.setEndian(std::endian::little);

    int32_t header;
    int32_t fileSize;
    int32_t type;

    ds >> header;
    ds >> fileSize;
    ds >> type;

    if (type == -0x5f4e3e00) {
        d_ptr->m_type = Type::VR_STATIC_VR;
    } else if (type == -0x5f4e3c00) {
        d_ptr->m_type = Type::VR_STATIC_PIC;
    } else {
        d_ptr->m_type = Type::VR_UNKNOWN;
    }

    // Parse image data
    d_ptr->m_fileVr.seekg(12);

    uint32_t imageSize;
    ds >> imageSize;

    uint32_t quality;
    ds >> quality;

    uint32_t dataSize;
    ds >> dataSize; // File size - 16

    std::vector<uint8_t> rawData(dataSize);
    ds.read(dataSize, rawData.data());

    // Unpack image
    Dct dct;
    bool ret = false;
    if (d_ptr->m_type == FvrVr::Type::VR_STATIC_VR) {
        ret = dct.unpack(rawData, quality, 256, 6144, d_ptr->m_rgb565Data);
    } else if (d_ptr->m_type == FvrVr::Type::VR_STATIC_PIC) {
        ret = dct.unpack(rawData, quality, 640, 480, d_ptr->m_rgb565Data);
    }

    if (!ret) {
        std::cerr << "Failed to unpack image" << std::endl;
        return false;
    }

    // Read animation data
    d_ptr->m_animationList.clear();
    if (!d_ptr->m_fileVr.eof()) {
        // Animation data
        while (!d_ptr->m_fileVr.eof()) {
            uint32_t animHeader;
            ds >> animHeader;

            if (animHeader != 0xa0b1c201) {
                break;
            }

            uint32_t animSize;
            ds >> animSize;

            char animName[0x20];
            ds.read(0x20, (uint8_t*)animName);
            std::string animNameStr(animName);
            std::transform(animNameStr.begin(), animNameStr.end(), animNameStr.begin(), ::tolower);

            uint32_t frameCount;
            ds >> frameCount;

            Animation animation;
            animation.name = animNameStr;
            for (int i = 0; i < frameCount; i++) {
                uint32_t frameHeader;
                ds >> frameHeader;

                if (frameHeader != 0xa0b1c211) {
                    std::cerr << "Invalid frame header" << std::endl;
                    return false;
                }

                uint32_t frameSize;
                ds >> frameSize;

                if (frameSize == 8) {
                    animation.frames.push_back({});
                    continue;
                }

                uint32_t blockCount;
                ds >> blockCount;

                std::vector<uint32_t> blockOffsetList(blockCount);
                for (int idxBlock = 0; idxBlock < blockCount; idxBlock++) {
                    uint32_t pixelOffset;
                    ds >> pixelOffset;

                    blockOffsetList[idxBlock] = pixelOffset;
                }

                // Frame image data
                uint32_t quality;
                ds >> quality;

                uint32_t dataSize;
                ds >> dataSize; // File size - 16

                std::vector<uint8_t> rawData(dataSize);
                ds.read(dataSize, rawData.data());

                Dct dct;
                std::vector<uint8_t> frameData;
                if (!dct.unpackBlock(blockCount, rawData, quality, frameData)) {
                    std::cerr << "Failed to unpack animation frame" << std::endl;
                    return false;
                }

                AnimationFrame frame { blockOffsetList, frameData };
                animation.frames.push_back(frame);
            }

            d_ptr->m_animationList.insert({ animNameStr, animation });
        }
    }

    return true;
}

void FvrVr::close()
{
    d_ptr->m_fileVr.close();
}

bool FvrVr::isOpen() const
{
    return d_ptr->m_fileVr.is_open();
}

int FvrVr::getWidth() const
{
    switch (d_ptr->m_type) {
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
    switch (d_ptr->m_type) {
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
    return d_ptr->m_type;
}

bool FvrVr::getImage(Image& image) const
{
    if (d_ptr->m_rgb565Data.empty()) {
        return false;
    }

    image.fromRgb565(d_ptr->m_rgb565Data, getWidth(), getHeight());

    return true;
}

bool FvrVr::getImageCubemap(Image& image) const
{
    if (getType() != Type::VR_STATIC_VR) {
        return false;
    }

    if (d_ptr->m_rgb565Data.empty()) {
        return false;
    }

    std::vector<uint8_t> cubemapRgb565Data;
    if (!d_ptr->rgb565DataToCubemap(d_ptr->m_rgb565Data, cubemapRgb565Data)) {
        return false;
    }

    image.fromRgb565(cubemapRgb565Data, 2048, 1536);

    return true;
}

const std::vector<uint8_t>& FvrVr::getData() const
{
    return d_ptr->m_rgb565Data;
}

const std::vector<uint8_t> FvrVr::getDataCrossCubemap() const
{
    if (getType() != Type::VR_STATIC_VR) {
        return {};
    }

    std::vector<uint8_t> cubemapRgb565Data;
    if (!d_ptr->rgb565DataToCubemap(d_ptr->m_rgb565Data, cubemapRgb565Data)) {
        return {};
    }

    return cubemapRgb565Data;
}

std::vector<std::string> FvrVr::getAnimationList() const
{
    std::vector<std::string> animList;
    for (const auto& anim : d_ptr->m_animationList) {
        animList.push_back(anim.first);
    }

    return animList;
}

bool FvrVr::getAnimation(const std::string& animName, Animation& animation) const
{
    auto it = d_ptr->m_animationList.find(animName);
    if (it == d_ptr->m_animationList.end()) {
        std::cerr << "Animation " << animName << " not found" << std::endl;
        return false;
    }

    animation = it->second;
    return true;
}

bool FvrVr::getAnimationCrossCubemap(const std::string& animName, Animation& animation) const
{
    if (getType() != Type::VR_STATIC_VR) {
        return false;
    }

    if (!getAnimation(animName, animation)) {
        return false;
    }

    for (auto& frame : animation.frames) {
        for (uint32_t& offset : frame.offsetList) {
            // Convert offset from 256x6144 original to 2048x1536 cross cubemap format
            size_t faceOffsets[6 * 4] = {
                2048u * 512u * 2u + 512u,
                2048u * 512u * 2u + 512u + 256u,
                2048u * 512u * 2u + 512u + 2048u * 256u + 256u,
                2048u * 512u * 2u + 512u + 2048u * 256u,

                2048u * 512u,
                2048u * 512u + 256u,
                2048u * 512u + 2048u * 256u + 256u,
                2048u * 512u + 2048u * 256u,

                512u,
                512u + 256u,
                512u + 2048u * 256u + 256u,
                512u + 2048u * 256u,

                2048u * 512u + 512u * 2u,
                2048u * 512u + 512u * 2u + 256u,
                2048u * 512u + 512u * 2u + 2048u * 256u + 256u,
                2048u * 512u + 512u * 2u + 2048u * 256u,

                2048u * 512u + 512u,
                2048u * 512u + 512u + 256u,
                2048u * 512u + 512u + 2048u * 256u + 256u,
                2048u * 512u + 512u + 2048u * 256u,

                2048u * 512u + 512u * 3u,
                2048u * 512u + 512u * 3u + 256u,
                2048u * 512u + 512u * 3u + 2048u * 256u + 256u,
                2048u * 512u + 512u * 3u + 2048u * 256u,
            };
            int mod = offset % (256 * 256);
            offset = faceOffsets[offset / (256 * 256)] + mod % 256 + mod / 256 * 2048;
        }
    }

    return true;
}

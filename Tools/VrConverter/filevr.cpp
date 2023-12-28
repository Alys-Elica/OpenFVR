#include "filevr.h"

#include <iostream>

FileVr::FileVr()
{
    m_type = FileVr::Type::VR_UNKNOWN;

    m_file.setEndian(File::Endian::LittleEndian);
}

FileVr::~FileVr()
{
    m_file.close();
}

bool FileVr::load(const std::string &filename)
{
    if (!m_file.open(filename, std::ios::binary | std::ios::in))
    {
        std::cout << "Failed to open file" << std::endl;
        return false;
    }

    int32_t header;
    int32_t fileSize;
    int32_t type;

    m_file >> header;
    m_file >> fileSize;
    m_file >> type;

    if (type == -0x5f4e3e00)
    {
        m_type = Type::VR_STATIC_VR;
    }
    else if (type == -0x5f4e3c00)
    {
        m_type = Type::VR_STATIC_PIC;
    }
    else
    {
        m_type = Type::VR_UNKNOWN;
    }

    // TODO: additional checks ?

    return isValid();
}

bool FileVr::isValid() const
{
    return m_type != Type::VR_UNKNOWN;
}

int FileVr::getFrameWidth() const
{
    if (!isValid())
    {
        std::cout << "File is invalid or not loaded" << std::endl;
        return 0;
    }

    if (m_type == Type::VR_STATIC_VR)
    {
        return 256;
    }
    else if (m_type == Type::VR_STATIC_PIC)
    {
        return 640;
    }

    return 0;
}

int FileVr::getFrameHeight() const
{
    if (!isValid())
    {
        std::cout << "File is invalid or not loaded" << std::endl;
        return 0;
    }

    if (m_type == Type::VR_STATIC_VR)
    {
        return 6144;
    }
    else if (m_type == Type::VR_STATIC_PIC)
    {
        return 480;
    }

    return 0;
}

FileVr::Type FileVr::getType() const
{
    return m_type;
}

bool FileVr::getRawFrameRgb565(ByteArray &rgb565Data)
{
    if (!isValid())
    {
        std::cout << "File is invalid or not loaded" << std::endl;
        return false;
    }
    if (!m_dct.isValid())
    {
        std::cout << "DCT is not valid" << std::endl;
        return false;
    }

    m_file.seek(12);

    uint32_t imageSize;
    m_file >> imageSize;

    uint32_t quality;
    uint32_t dataSize;

    m_file >> quality;
    m_file >> dataSize; // File size - 16

    ByteArray rawData(dataSize);
    m_file.read((char *)rawData.data(), dataSize);

    // Unpack image
    bool ret = false;
    if (m_type == Type::VR_STATIC_VR)
    {
        ret = m_dct.unpackVr(rawData, quality, rgb565Data);
    }
    else if (m_type == Type::VR_STATIC_PIC)
    {
        ret = m_dct.unpackPicture(rawData, quality, rgb565Data);
    }

    return ret;
}

bool FileVr::getRawAnimationsRgba32(std::vector<Animation> &animationList)
{
    if (!isValid())
    {
        std::cout << "File is invalid or not loaded" << std::endl;
        return false;
    }

    m_file.seek(12);

    uint32_t part1Size;
    m_file >> part1Size;

    m_file.seek(8 + part1Size);

    if (m_file.atEnd())
    {
        std::cout << "No animation data" << std::endl;
        return false;
    }

    while (!m_file.atEnd())
    {
        Animation animation;
        if (!readAnimation(animation))
        {
            continue;
        }

        animationList.push_back(animation);
    }

    return true;
}

bool FileVr::readAnimation(Animation &animation)
{
    uint32_t animHeader;
    m_file >> animHeader;

    if (animHeader != 0xa0b1c201)
    {
        return false;
    }

    uint32_t animSize;
    m_file >> animSize;

    char animName[0x20];
    m_file.read(animName, 0x20);

    uint32_t animFrameCount;
    m_file >> animFrameCount;

    animation.name = animName;
    if (m_type == Type::VR_STATIC_PIC)
    {
        animation.width = 640;
        animation.height = 480;
    }
    else if (m_type == Type::VR_STATIC_VR)
    {
        animation.width = 256;
        animation.height = 6144;
    }
    // Read frames
    for (uint32_t frameIdx = 0; frameIdx < animFrameCount; ++frameIdx)
    {
        Frame frame;
        if (readAnimationFrame(frame))
        {
            animation.frameList.push_back(frame);
        }
    }

    return true;
}

bool FileVr::readAnimationFrame(Frame &frame)
{
    uint32_t frameHeader;
    m_file >> frameHeader;

    if (frameHeader != 0xa0b1c211)
    {
        std::cout << "Invalid frame header" << std::endl;
        return false;
    }

    uint32_t frameSize;
    m_file >> frameSize;

    if (frameSize <= 8)
    {
        std::cout << "Empty frame" << std::endl;

        if (m_type == Type::VR_STATIC_PIC)
        {
            frame.resize(640, 480);
        }
        else if (m_type == Type::VR_STATIC_VR)
        {
            frame.resize(256, 6144);
        }

        return true;
    }

    uint32_t blockCount;
    m_file >> blockCount;

    std::vector<uint32_t> blockStartList(blockCount);
    for (uint32_t j = 0; j < blockCount; ++j)
    {
        m_file >> blockStartList[j];
    }

    uint32_t quality;
    uint32_t dataSize;

    m_file >> quality;
    m_file >> dataSize;

    ByteArray rawData(dataSize);
    m_file.read((char *)rawData.data(), dataSize);

    ByteArray blockData;
    bool ret = m_dct.unpackBlock(blockCount, rawData, quality, blockData);

    if (m_type == Type::VR_STATIC_PIC)
    {
        frame.resize(64, 480);
    }
    else if (m_type == Type::VR_STATIC_VR)
    {
        frame.resize(256, 6144);
    }

    int dataIdx = 0;
    for (size_t blockIdx = 0; blockIdx < blockStartList.size(); ++blockIdx)
    {
        uint32_t blockStart = blockStartList[blockIdx];

        blockStart *= 4;

        for (int i = 0; i < 64; ++i)
        {
            int tmp = blockStart + (i % 8) * 4 + (i / 8) * 256 * 4;
            frame.data()[tmp + 0] = blockData[dataIdx + 2]; // Red
            frame.data()[tmp + 1] = blockData[dataIdx + 1]; // Green
            frame.data()[tmp + 2] = blockData[dataIdx + 0]; // Blue
            frame.data()[tmp + 3] = 0xFF;                   // Alpha

            dataIdx += 3;
        }
    }

    return true;
}

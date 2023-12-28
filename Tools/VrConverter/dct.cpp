#include "dct.h"

#include <iostream>

#include <windows.h>

// Private implementation
class DctPrivate
{
    friend class Dct;

public:
    DctPrivate()
    {
        libDct = LoadLibrary(TEXT("dct.dll"));
        if (!libDct)
        {
            std::cout << "Failed to load dct.dll" << std::endl;
            return;
        }

        // Unpack image functions
        DCT_Unpack_640_480 = (DCT_Unpack_Image_t)GetProcAddress(libDct, "?Unpack_Picture_Rgb_15_16_640_480@@YAXPAGPAEHH@Z");
        DCT_Unpack_256_6144 = (DCT_Unpack_Image_t)GetProcAddress(libDct, "?Unpack_Picture_Rgb_15_16_256_6144@@YAXPAGPAEHH@Z");
        if (!DCT_Unpack_640_480 || !DCT_Unpack_256_6144)
        {
            std::cout << "Failed to get function pointer" << std::endl;
            clean();
            return;
        }

        DCT_init = (DCT_cdtor)GetProcAddress(libDct, "??0DCT@@QAE@XZ");
        DCT_deinit = (DCT_cdtor)GetProcAddress(libDct, "??1DCT@@QAE@XZ");
        if (!DCT_init || !DCT_deinit)
        {
            std::cout << "Failed to get DCT const/dest function pointer" << std::endl;
            clean();
            return;
        }

        DCT_Unpack_Block = (DCT_Unpack_Block_t)GetProcAddress(libDct, "?Unpack_Bloc_Rgb_24@DCT@@QAEXPAEH0HH@Z");
        if (!DCT_Unpack_Block)
        {
            std::cout << "Failed to get function pointer" << std::endl;
            clean();
            return;
        }

        // DCT object size
        m_dctObj = static_cast<uint8_t *>(malloc(750'000));

        DCT_init(m_dctObj);
    }

    ~DctPrivate()
    {
        clean();
    }

    void clean()
    {
        if (m_dctObj)
        {
            if (DCT_deinit)
            {
                DCT_deinit(m_dctObj);
            }
            free(m_dctObj);
        }

        m_dctObj = NULL;

        FreeLibrary(libDct);
        libDct = NULL;

        DCT_init = NULL;
        DCT_deinit = NULL;

        DCT_Unpack_640_480 = NULL;
        DCT_Unpack_256_6144 = NULL;
    }

    bool isValid()
    {
        return libDct != NULL;
    }

private:
    typedef void(__thiscall *DCT_cdtor)(void *thisptr);
    typedef void(__cdecl *DCT_Unpack_Image_t)(uint8_t *outData, const uint8_t *data, int dataSize, int quality);
    typedef void(__thiscall *DCT_Unpack_Block_t)(void *thisptr, unsigned char *blockData, int blockCount, unsigned char *data, int dataSize, int quality);

private:
    HINSTANCE libDct = NULL;

    DCT_cdtor DCT_init = NULL;
    DCT_cdtor DCT_deinit = NULL;

    DCT_Unpack_Image_t DCT_Unpack_640_480 = NULL;
    DCT_Unpack_Image_t DCT_Unpack_256_6144 = NULL;

    DCT_Unpack_Block_t DCT_Unpack_Block = NULL;

    uint8_t *m_dctObj = NULL;
};

// Public implementation
Dct::Dct()
{
    d_ptr = new DctPrivate();
}

Dct::~Dct()
{
    delete d_ptr;
}

bool Dct::isValid()
{
    return true;
}

bool Dct::unpackPicture(
    const ByteArray &imageData,
    const int quality,
    ByteArray &rgb565Data)
{
    if (!isValid())
    {
        std::cout << "DCT is invalid" << std::endl;
        return false;
    }

    rgb565Data.resize(640 * 480 * 2);

    d_ptr->DCT_Unpack_640_480(rgb565Data.data(), imageData.data(), imageData.size(), quality);

    return true;
}

bool Dct::unpackVr(
    const ByteArray &imageData,
    const int quality,
    ByteArray &rgb565Data)
{
    if (!isValid())
    {
        std::cout << "DCT is invalid" << std::endl;
        return false;
    }

    rgb565Data.resize(256 * 6144 * 2);

    d_ptr->DCT_Unpack_256_6144(rgb565Data.data(), imageData.data(), imageData.size(), quality);

    return true;
}

bool Dct::unpackBlock(
    const int blockCount,
    const ByteArray &imageData,
    const int quality,
    ByteArray &outData)
{
    if (!isValid())
    {
        std::cout << "DCT is invalid" << std::endl;
        return false;
    }

    outData.resize(64 * 3 * blockCount);

    d_ptr->DCT_Unpack_Block(d_ptr->m_dctObj, outData.data(), blockCount, const_cast<uint8_t *>(imageData.data()), imageData.size(), quality);

    return true;
}

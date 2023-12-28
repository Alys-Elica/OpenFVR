#include "frame.h"

#include <algorithm>
#include <iostream>

#include <png.h>
#include <webp/encode.h>
#include <webp/mux.h>

#include "file.h"

// Frame class
Frame Frame::rgb565ToFrame(
    const ByteArray &rgb565Data,
    int width,
    int height)
{
    if (rgb565Data.empty() || width <= 0 || height <= 0)
    {
        std::cout << "Invalid parameters" << std::endl;
        return Frame();
    }

    if (rgb565Data.size() != width * height * 2)
    {
        std::cout << "Data size and frame dimenyions does not correspond" << std::endl;
        return Frame();
    }

    Frame frame(width, height);

    int outIdx = 0;
    for (size_t i = 0; i < rgb565Data.size(); i += 2)
    {
        uint16_t pix = rgb565Data.data()[i] | (rgb565Data.data()[i + 1] << 8);

        // R5 G6 B5 -> R8 G8 B8
        frame.data()[outIdx++] = (pix & 0b1111100000000000) >> 8;
        frame.data()[outIdx++] = (pix & 0b0000011111100000) >> 3;
        frame.data()[outIdx++] = (pix & 0b0000000000011111) << 3;
        frame.data()[outIdx++] = 0xFF;
    }

    return frame;
}

bool Frame::vrToCubicMap(
    const Frame &inFrame,
    Frame &outFrame)
{
    if (inFrame.width() != 256 || inFrame.height() != 6144)
    {
        std::cout << "Invalid image size" << std::endl;
        return false;
    }

    outFrame.resize(2048, 1536);
    size_t faceOffsets[6] = {
        2048u * 512u * 2u * 4u + 512u * 4u,
        2048u * 512u * 4u,
        512u * 4u,
        2048u * 512u * 4u + 512u * 4u * 2u,
        2048u * 512u * 4u + 512u * 4u,
        2048u * 512u * 4u + 512u * 4u * 3u,
    };

    for (int faceIdx = 0; faceIdx < 6; ++faceIdx)
    {
        uint32_t faceOffset = faceOffsets[faceIdx];

        for (int lineIdx = 0; lineIdx < 256; ++lineIdx)
        {
            std::memcpy(
                outFrame.data() + faceOffset + lineIdx * 2048 * 4,
                inFrame.data() + faceIdx * 512 * 512 * 4 + lineIdx * 256 * 4,
                256 * 4);
            std::memcpy(
                outFrame.data() + faceOffset + 256 * 4 + lineIdx * 2048 * 4,
                inFrame.data() + faceIdx * 512 * 512 * 4 + 256 * 256 * 4 + lineIdx * 256 * 4,
                256 * 4);
            std::memcpy(
                outFrame.data() + faceOffset + 2048 * 256 * 4 + lineIdx * 2048 * 4,
                inFrame.data() + faceIdx * 512 * 512 * 4 + 3 * 256 * 256 * 4 + lineIdx * 256 * 4,
                256 * 4);
            std::memcpy(
                outFrame.data() + faceOffset + 256 * 4 + 2048 * 256 * 4 + lineIdx * 2048 * 4,
                inFrame.data() + faceIdx * 512 * 512 * 4 + 2 * 256 * 256 * 4 + lineIdx * 256 * 4,
                256 * 4);
        }
    }

    return true;
}

bool Frame::framesToWebP(
    const std::vector<Frame> &frameList,
    const std::string &filename)
{
    if (frameList.empty())
    {
        std::cout << "No frame available" << std::endl;
        return false;
    }

    if (filename.empty())
    {
        std::cout << "Invalid filename" << std::endl;
        return false;
    }

    int width = frameList[0].width();
    int height = frameList[0].height();
    for (const Frame &frame : frameList)
    {
        if (!frame.isValid())
        {
            std::cout << "Invalid frame" << std::endl;
            return false;
        }

        if (frame.width() != width || frame.height() != height)
        {
            std::cout << "Frame dimensions does not correspond" << std::endl;
            return false;
        }
    }

    WebPAnimEncoderOptions encOptions;
    WebPAnimEncoderOptionsInit(&encOptions);

    encOptions.anim_params.loop_count = 0; // 0 = infinite loop.
    encOptions.anim_params.bgcolor = 0;    // 0 = transparent background.
    encOptions.minimize_size = 1;

    const int durationMs = 8;

    WebPConfig config;
    WebPConfigInit(&config);

    config.lossless = 1;

    WebPAnimEncoder *enc = WebPAnimEncoderNew(width, height, &encOptions);
    for (const Frame &frame : frameList)
    {
        if (!frame.isValid())
        {
            std::cout << "Invalid frame" << std::endl;
            continue;
        }

        WebPPicture pic;
        if (!WebPPictureInit(&pic))
        {
            WebPAnimEncoderDelete(enc);
            std::cout << "Failed to initialize picture" << std::endl;
            return false;
        }

        pic.width = frame.width();
        pic.height = frame.height();

        if (!WebPPictureAlloc(&pic))
        {
            WebPPictureFree(&pic);
            WebPAnimEncoderDelete(enc);
            std::cout << "Failed to allocate picture" << std::endl;
            return false;
        }

        WebPPictureImportRGBA(&pic, frame.data(), frame.width() * 4);
        WebPAnimEncoderAdd(enc, &pic, durationMs, &config);
        WebPPictureFree(&pic);
    }

    WebPData webpData;

    WebPAnimEncoderAdd(enc, NULL, durationMs, &config);
    WebPAnimEncoderAssemble(enc, &webpData);
    WebPAnimEncoderDelete(enc);

    File file;
    if (!file.open(filename, std::ios::out | std::ios::binary))
    {
        std::cout << "Failed to open file for writing" << std::endl;
        return false;
    }

    file.write(webpData.bytes, webpData.size);
    file.close();

    return true;
}

Frame::Frame(int width, int height)
{
    resize(width, height);
}

Frame::~Frame()
{
}

bool Frame::isValid() const
{
    return m_width > 0 && m_height > 0 && !m_data.empty();
}

int Frame::width() const
{
    return m_width;
}

int Frame::height() const
{
    return m_height;
}

bool Frame::resize(int width, int height)
{
    if (width <= 0 || height <= 0)
    {
        return false;
    }

    m_width = width;
    m_height = height;
    m_data.resize(width * height * 4);

    clear();

    return true;
}

void Frame::clear()
{
    std::fill(m_data.begin(), m_data.end(), 0);
}

bool Frame::saveToWebp(const std::string &fileName) const
{
    if (!isValid())
    {
        return false;
    }

    uint8_t *output = nullptr;
    size_t outSize = WebPEncodeLosslessRGBA(
        data(),
        width(),
        height(),
        width() * 4,
        &output);

    if (outSize == 0 || output == nullptr)
    {
        std::cout << "Failed to encode image" << std::endl;
        return false;
    }

    File file;
    if (!file.open(fileName, std::ios::out | std::ios::binary))
    {
        std::cout << "Failed to open file for writing" << std::endl;
        delete[] output;
        return false;
    }

    file.write(output, outSize);
    file.close();

    delete[] output;

    return true;
}

bool Frame::saveToPng(const std::string &fileName) const
{
    if (width() == 0 || height() == 0)
    {
        std::cout << "Image has zero width or height" << std::endl;
        return false;
    }

    FILE *fp = NULL;
    fopen_s(&fp, fileName.c_str(), "wb");
    if (!fp)
    {
        std::cout << "Failed to open image file for writing" << std::endl;
        return false;
    }

    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png)
    {
        fclose(fp);
        std::cout << "Failed to create PNG write structure" << std::endl;
        return false;
    }

    png_infop info = png_create_info_struct(png);
    if (!info)
    {
        fclose(fp);
        png_destroy_write_struct(&png, NULL);

        std::cout << "Failed to create PNG info structure" << std::endl;
        return false;
    }

    if (setjmp(png_jmpbuf(png)))
    {
        fclose(fp);
        png_destroy_write_struct(&png, &info);

        std::cout << "Failed to set PNG jump buffer" << std::endl;
        return false;
    }

    png_init_io(png, fp);

    // Output is 8bit depth, RGBA format.
    png_set_IHDR(
        png,
        info,
        width(), height(),
        8,
        PNG_COLOR_TYPE_RGBA,
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_DEFAULT,
        PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png, info);

    // To remove the alpha channel for PNG_COLOR_TYPE_RGB format,
    // Use png_set_filler().
    // png_set_filler(png, 0, PNG_FILLER_AFTER);

    png_bytepp rows = (png_bytepp)png_malloc(png, height() * sizeof(png_bytep));
    for (int y = 0; y < height(); ++y)
    {
        rows[y] = (png_bytep)png_malloc(png, sizeof(uint8_t) * width() * 4);
        for (int x = 0; x < width(); ++x)
        {
            rows[y][x * 4 + 0] = m_data[y * width() * 4 + x * 4 + 0];
            rows[y][x * 4 + 1] = m_data[y * width() * 4 + x * 4 + 1];
            rows[y][x * 4 + 2] = m_data[y * width() * 4 + x * 4 + 2];
            rows[y][x * 4 + 3] = m_data[y * width() * 4 + x * 4 + 3];
        }
    }

    png_write_image(png, rows);
    png_write_end(png, NULL);

    // Cleanup heap allocation
    for (int y = 0; y < height(); ++y)
    {
        free(rows[y]);
    }
    free(rows);

    fclose(fp);
    png_destroy_write_struct(&png, &info);

    return true;
}

const uint8_t *Frame::data() const
{
    return m_data.data();
}

uint8_t *Frame::data()
{
    return m_data.data();
}

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#include <png.h>

bool saveImagePng(const std::vector<uint8_t> &data,
                  const unsigned int width, const unsigned int height, const std::string &fileName)
{
    std::cout << "Saving " << fileName << std::endl;

    FILE *file = NULL;
    fopen_s(&file, fileName.c_str(), "wb");

    if (!file)
    {
        std::cerr << "Failed to open " << fileName << std::endl;
        return false;
    }

    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png_ptr)
    {
        std::cerr << "Failed to create PNG write struct" << std::endl;

        fclose(file);

        return false;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        std::cerr << "Failed to create PNG info struct" << std::endl;

        png_destroy_write_struct(&png_ptr, nullptr);
        fclose(file);

        return false;
    }

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        std::cerr << "Failed to set PNG jump buffer" << std::endl;

        png_destroy_write_struct(&png_ptr, &info_ptr);
        fclose(file);

        return false;
    }

    png_init_io(png_ptr, file);

    png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    png_write_info(png_ptr, info_ptr);

    png_bytep *row_pointers = new png_bytep[height];
    for (unsigned int y = 0; y < height; y++)
    {
        row_pointers[y] = new png_byte[width * 3];
        for (unsigned int x = 0; x < width; x++)
        {
            row_pointers[y][x * 3 + 0] = data[(y * width + x) * 3 + 0];
            row_pointers[y][x * 3 + 1] = data[(y * width + x) * 3 + 1];
            row_pointers[y][x * 3 + 2] = data[(y * width + x) * 3 + 2];
        }
    }

    png_write_image(png_ptr, row_pointers);
    png_write_end(png_ptr, nullptr);

    // Cleanup
    fclose(file);
    for (unsigned int y = 0; y < height; y++)
    {
        delete[] row_pointers[y];
    }
    delete[] row_pointers;

    png_destroy_write_struct(&png_ptr, &info_ptr);

    return true;
}

void buildImage(const uint8_t palette[0x300], const std::vector<uint8_t> &data, std::vector<uint8_t> &image)
{
    image.resize(data.size() * 3);

    for (size_t i = 0; i < data.size(); i++)
    {
        image[i * 3 + 0] = palette[data[i] * 3 + 0];
        image[i * 3 + 1] = palette[data[i] * 3 + 1];
        image[i * 3 + 2] = palette[data[i] * 3 + 2];
    }
}

void rleDecompress(const std::vector<uint8_t> &data, std::vector<uint8_t> &dest)
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

bool mtrConvert(const std::string &mtrFileName)
{
    std::cout << "Converting " << mtrFileName << std::endl;

    std::fstream file(mtrFileName, std::ios_base::in | std::ios_base::binary);

    if (!file.is_open())
    {
        std::cerr << "Failed to open " << mtrFileName << std::endl;
        return false;
    }

    // Check magic
    file.seekp(0);
    std::string magic(5, '\0');
    file.read(magic.data(), 5);
    if (magic != "MDRAW")
    {
        std::cerr << "Invalid magic" << std::endl;
        return false;
    }

    // Check type
    file.seekp(0x15);
    std::string type(2, '\0');
    file.read(type.data(), 2);
    if (type != "NC" && type != "FC")
    {
        std::cerr << "Invalid type" << std::endl;
        return false;
    }

    // Read width
    file.seekp(0x06);
    std::string strWidth(4, '\0');
    file.read(strWidth.data(), 4);

    unsigned int width;
    std::istringstream(strWidth) >> std::hex >> width;
    ++width; // Width is 1 less than actual width

    // Read height
    file.seekp(0x0B);
    std::string strHeight(4, '\0');
    file.read(strHeight.data(), 4);

    unsigned int height;
    std::istringstream(strHeight) >> std::hex >> height;
    ++height; // Height is 1 less than actual height

    if (width == 0 || height == 0 || width > 1023 || height > 1023)
    {
        std::cerr << "Invalid size" << std::endl;
        return false;
    }

    // Read size
    file.seekp(0x18);
    std::string strSize(8, '\0');
    file.read(strSize.data(), 8);

    unsigned int size;
    std::istringstream(strSize) >> std::hex >> size;

    file.seekp(0, std::ios_base::end);
    if (size != file.tellp())
    {
        std::cerr << "Invalid size" << std::endl;
        return false;
    }

    // Read palette
    file.seekp(0x32);
    uint8_t palette[0x300]; // 256 RGB 24 bits colors
    file.read(reinterpret_cast<char *>(palette), 0x300);

    // Read data
    file.seekp(0x10);
    std::string tmp(4, '\0');
    file.read(tmp.data(), 4);
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
            file.seekp(-8, std::ios_base::end);
            std::streampos end = file.tellp();

            // Size minus header (0x32 bytes), palette (0x300 bytes) and reserved end string (8 bytes)
            int dataSize = size - 0x32 - 0x300 - 8;
            std::vector<uint8_t> source(dataSize);
            file.seekp(0x332);
            file.read((char *)source.data(), dataSize);

            std::vector<uint8_t> decompressed;
            decompressed.reserve(width * height);

            rleDecompress(source, decompressed);

            if (decompressed.size() != width * height)
            {
                std::cerr << "Invalid decompressed size" << std::endl;
                return false;
            }

            std::vector<uint8_t> image;
            buildImage(palette, decompressed, image);

            return saveImagePng(image, width, height, mtrFileName + ".png");
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

    return true;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <mtr file> [mtr file] ..." << std::endl;
        return false;
    }

    for (int i = 1; i < argc; i++)
    {
        std::string mtrFileName = argv[i];

        mtrConvert(mtrFileName);
    }
}

#include "image.h"

#include <iostream>
#include <vector>

#include <png.h>

#if !defined(_WIN32) && !defined(__STDC_WANT_LIB_EXT1__)
#include <assert.h>

typedef int errno_t;

errno_t fopen_s(FILE** f, const char* name, const char* mode)
{
    errno_t ret = 0;
    assert(f);
    *f = fopen(name, mode);
    /* Can't be sure about 1-to-1 mapping of errno and MS' errno_t */
    if (!*f)
        ret = errno;
    return ret;
}
#endif

/* Private */
class Image::ImagePrivate {
    friend class Image;

private:
    int width = 0;
    int height = 0;
    std::vector<Image::Pixel> pixels;
};

/* Public */
Image::Image(int width, int height)
{
    d_ptr = new ImagePrivate();

    resize(width, height);
}

Image::~Image()
{
    delete d_ptr;
}

bool Image::isValid() const
{
    return d_ptr->width > 0 && d_ptr->height > 0;
}

int Image::width() const
{
    return d_ptr->width;
}

int Image::height() const
{
    return d_ptr->height;
}

bool Image::resize(int width, int height)
{
    if (width <= 0 || height <= 0) {
        return false;
    }

    d_ptr->width = width;
    d_ptr->height = height;
    d_ptr->pixels.resize(width * height);

    return true;
}

void Image::clear()
{
    d_ptr->width = 0;
    d_ptr->height = 0;
    d_ptr->pixels.clear();
}

void Image::setPixel(int x, int y, const Pixel& pixel)
{
    if (!isValid()) {
        return;
    }

    if (x < 0 || x >= d_ptr->width || y < 0 || y >= d_ptr->height) {
        return;
    }

    d_ptr->pixels[y * d_ptr->width + x] = pixel;
}

Image::Pixel Image::pixel(int x, int y) const
{
    if (!isValid()) {
        return {};
    }

    if (x < 0 || x >= d_ptr->width || y < 0 || y >= d_ptr->height) {
        return {};
    }

    return d_ptr->pixels[y * d_ptr->width + x];
}

bool Image::fromRgb565(const std::vector<uint8_t>& rgb565Data, int width, int height)
{
    if (width <= 0 || height <= 0) {
        std::cerr << "Invalid image size" << std::endl;
        return false;
    }

    if (!resize(width, height)) {
        std::cerr << "Failed to resize image" << std::endl;
        return false;
    }

    if (rgb565Data.size() != width * height * 2) {
        std::cerr << "Invalid image data size" << std::endl;
        return false;
    }

    for (int i = 0; i < width * height; ++i) {
        const uint16_t pixel = rgb565Data[i * 2 + 0] | (rgb565Data[i * 2 + 1] << 8);

        Pixel& pix = d_ptr->pixels[i];
        pix.r = (pixel & 0b1111100000000000) >> 8;
        pix.g = (pixel & 0b0000011111100000) >> 3;
        pix.b = (pixel & 0b0000000000011111) << 3;
        pix.a = 0xFF;
    }

    return true;
}

bool Image::toRgb565(std::vector<uint8_t>& rgb565Data) const
{
    if (!isValid()) {
        return false;
    }

    rgb565Data.resize(d_ptr->width * d_ptr->height * 2);

    for (int i = 0; i < d_ptr->width * d_ptr->height; ++i) {
        const Pixel& pix = d_ptr->pixels[i];

        const uint16_t pixel = ((pix.r & 0b11111000) << 8) | ((pix.g & 0b11111100) << 3) | ((pix.b & 0b11111000) >> 3);

        rgb565Data[i * 2 + 0] = pixel & 0xFF;
        rgb565Data[i * 2 + 1] = (pixel >> 8) & 0xFF;
    }

    return true;
}

bool Image::savePng(const std::string& fileName) const
{
    std::cout << "Saving " << fileName << std::endl;

    FILE* file = NULL;
    fopen_s(&file, fileName.c_str(), "wb");

    if (!file) {
        std::cerr << "Failed to open " << fileName << std::endl;
        return false;
    }

    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png_ptr) {
        std::cerr << "Failed to create PNG write struct" << std::endl;

        fclose(file);

        return false;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        std::cerr << "Failed to create PNG info struct" << std::endl;

        png_destroy_write_struct(&png_ptr, nullptr);
        fclose(file);

        return false;
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        std::cerr << "Failed to set PNG jump buffer" << std::endl;

        png_destroy_write_struct(&png_ptr, &info_ptr);
        fclose(file);

        return false;
    }

    png_init_io(png_ptr, file);

    png_set_IHDR(png_ptr, info_ptr, d_ptr->width, d_ptr->height, 8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    png_write_info(png_ptr, info_ptr);

    png_bytep* row_pointers = new png_bytep[d_ptr->height];
    for (unsigned int y = 0; y < d_ptr->height; y++) {
        row_pointers[y] = new png_byte[d_ptr->width * 4];
        for (unsigned int x = 0; x < d_ptr->width; x++) {
            const Pixel pix = pixel(x, y);

            row_pointers[y][x * 4 + 0] = pix.r;
            row_pointers[y][x * 4 + 1] = pix.g;
            row_pointers[y][x * 4 + 2] = pix.b;
            row_pointers[y][x * 4 + 3] = pix.a;
        }
    }

    png_write_image(png_ptr, row_pointers);
    png_write_end(png_ptr, nullptr);

    // Cleanup
    fclose(file);
    for (unsigned int y = 0; y < d_ptr->height; y++) {
        delete[] row_pointers[y];
    }
    delete[] row_pointers;

    png_destroy_write_struct(&png_ptr, &info_ptr);

    return true;
}
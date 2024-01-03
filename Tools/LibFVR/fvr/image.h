#ifndef IMAGE_H
#define IMAGE_H

#include "libfvr_globals.h"

#include <string>
#include <vector>

class ImagePrivate;
class LIBFVR_EXPORT Image
{
public:
    struct Pixel
    {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;
    };

public:
    Image(int width = 0, int height = 0);
    ~Image();

    bool isValid() const;

    int width() const;
    int height() const;

    bool resize(int width, int height);
    void clear();

    void setPixel(int x, int y, const Pixel &pixel);
    Pixel pixel(int x, int y) const;

    bool fromRgb565(const std::vector<uint8_t> &rgb565Data, int width, int height);
    bool toRgb565(std::vector<uint8_t> &rgb565Data) const;

    bool savePng(const std::string &fileName) const;

private:
    ImagePrivate *d_ptr;
};

#endif // IMAGE_H

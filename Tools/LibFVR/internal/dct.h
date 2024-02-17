#ifndef DCT_H
#define DCT_H

#include <cstdint>
#include <vector>

class DctPrivate;
class Dct final
{

public:
    Dct();
    ~Dct();

    Dct(const Dct &) = delete;
    Dct &operator=(const Dct &) = delete;

    bool unpack(
        const std::vector<uint8_t> &imageData,
        const int quality,
        const int width,
        const int height,
        std::vector<uint8_t> &rgb565Data);
    bool unpackBlock(
        const int blockCount,
        const std::vector<uint8_t> &imageData,
        const int quality,
        std::vector<uint8_t> &outData);

private:
    DctPrivate *d_ptr;
};

#endif // DCT_H
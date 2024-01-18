#ifndef DCT_H
#define DCT_H

#include <cstdint>
#include <vector>

// TODO: this will be replaced with a proper implementation.
// It currently relies on the DLL from the original FVR engine.
class DctPrivate;
class Dct final
{

public:
    Dct();
    ~Dct();

    Dct(const Dct &) = delete;
    Dct &operator=(const Dct &) = delete;

    bool isValid();

    bool unpackPicture(
        const std::vector<uint8_t> &imageData,
        const int quality,
        std::vector<uint8_t> &rgb565Data);
    bool unpackVr(
        const std::vector<uint8_t> &imageData,
        const int quality,
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
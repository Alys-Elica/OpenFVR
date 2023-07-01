#ifndef DCT_H
#define DCT_H

#include <cstdint>
#include <vector>

#include "frame.h"

class DctPrivate;
class Dct
{

public:
    Dct();
    ~Dct();

    Dct(const Dct &) = delete;
    Dct &operator=(const Dct &) = delete;

    bool isValid();

    bool unpackPicture(
        const ByteArray &imageData,
        const int quality,
        ByteArray &rgb565Data);
    bool unpackVr(
        const ByteArray &imageData,
        const int quality,
        ByteArray &rgb565Data);
    bool unpackBlock(
        const int blockCount,
        const ByteArray &imageData,
        const int quality,
        ByteArray &outData);

private:
    DctPrivate *d_ptr;
};

#endif // DCT_H
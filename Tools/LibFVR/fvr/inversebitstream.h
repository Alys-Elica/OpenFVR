#ifndef INVERSEBITSTREAM_H
#define INVERSEBITSTREAM_H

#include "libfvr_globals.h"

#include <cstdint>
#include <vector>

class LIBFVR_EXPORT InverseBitstream
{
public:
    InverseBitstream();
    ~InverseBitstream();

    void setData(const std::vector<uint8_t> &data);
    int8_t next(uint8_t bits);

private:
    class InverseBitstreamPrivate;
    InverseBitstreamPrivate *d_ptr;
};

#endif // INVERSEBITSTREAM_H

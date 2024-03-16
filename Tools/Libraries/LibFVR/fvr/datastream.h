#ifndef DATASTREAM_H
#define DATASTREAM_H

#include "libfvr_globals.h"

#include <bit>
#include <cstdint>
#include <vector>

class LIBFVR_EXPORT DataStream {
public:
    DataStream();
    ~DataStream();

    void setData(const std::vector<uint8_t>& data);
    void setEndian(const std::endian& endian);

    // Read methods
    void read(std::vector<uint8_t>& data, const size_t size);

    DataStream& operator>>(int8_t& data);
    DataStream& operator>>(int16_t& data);
    DataStream& operator>>(int32_t& data);
    DataStream& operator>>(int64_t& data);

    DataStream& operator>>(uint8_t& data);
    DataStream& operator>>(uint16_t& data);
    DataStream& operator>>(uint32_t& data);
    DataStream& operator>>(uint64_t& data);

private:
    class DataStreamPrivate;
    DataStreamPrivate* d_ptr;
};

#endif // DATASTREAM_H

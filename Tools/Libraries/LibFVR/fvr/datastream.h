#ifndef DATASTREAM_H
#define DATASTREAM_H

#include "libfvr_globals.h"

#include <bit>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>

class LIBFVR_EXPORT DataStream {
public:
    DataStream(std::ifstream* file);
    DataStream(std::ofstream* file);
    DataStream(std::vector<uint8_t>* data);
    ~DataStream();

    void setEndian(const std::endian& endian);

    // Read methods
    void read(const size_t size, uint8_t* data);

    DataStream& operator>>(uint8_t& data);
    DataStream& operator>>(uint16_t& data);
    DataStream& operator>>(uint32_t& data);
    DataStream& operator>>(uint64_t& data);

    DataStream& operator>>(int8_t& data);
    DataStream& operator>>(int16_t& data);
    DataStream& operator>>(int32_t& data);
    DataStream& operator>>(int64_t& data);

    DataStream& operator>>(float& data);
    DataStream& operator>>(double& data);

    // Write methods
    void write(const size_t size, const uint8_t* data);

    DataStream& operator<<(const uint8_t data);
    DataStream& operator<<(const uint16_t data);
    DataStream& operator<<(const uint32_t data);
    DataStream& operator<<(const uint64_t data);

    DataStream& operator<<(const int8_t data);
    DataStream& operator<<(const int16_t data);
    DataStream& operator<<(const int32_t data);
    DataStream& operator<<(const int64_t data);

    DataStream& operator<<(const float data);
    DataStream& operator<<(const double data);

private:
    DataStream();
    class DataStreamPrivate;
    DataStreamPrivate* d_ptr;
};

#endif // DATASTREAM_H

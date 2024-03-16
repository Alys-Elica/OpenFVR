#include "datastream.h"

#include <cstring>
#include <iostream>

/* Private */
class DataStream::DataStreamPrivate {
    friend class DataStream;

public:
    uint8_t read8();
    uint16_t read16();
    uint32_t read32();
    uint64_t read64();

private:
    std::vector<uint8_t> m_data;
    std::endian m_endian;
    int m_index = -1;
};

uint8_t DataStream::DataStreamPrivate::read8()
{
    if (m_index + sizeof(uint8_t) > m_data.size()) {
        std::cerr << "Out of range" << std::endl;
        return 0;
    }

    return m_data[m_index++];
}

uint16_t DataStream::DataStreamPrivate::read16()
{
    if (m_index + sizeof(uint16_t) > m_data.size()) {
        std::cerr << "Out of range" << std::endl;
        return 0;
    }

    uint16_t data = 0;
    if (m_endian == std::endian::little) {
        data = read8();
        data |= read8() << 8;
    } else {
        data = read8() << 8;
        data |= read8();
    }

    return data;
}

uint32_t DataStream::DataStreamPrivate::read32()
{
    if (m_index + sizeof(uint32_t) > m_data.size()) {
        std::cerr << "Out of range" << std::endl;
        return 0;
    }

    uint32_t data = 0;
    if (m_endian == std::endian::little) {
        data = read16();
        data |= read16() << 16;
    } else {
        data = read16() << 16;
        data |= read16();
    }

    return data;
}

uint64_t DataStream::DataStreamPrivate::read64()
{
    if (m_index + sizeof(uint64_t) > m_data.size()) {
        std::cerr << "Out of range" << std::endl;
        return 0;
    }

    uint64_t data = 0;
    if (m_endian == std::endian::little) {
        data = read32();
        data |= static_cast<uint64_t>(read32()) << 32;
    } else {
        data = static_cast<uint64_t>(read32()) << 32;
        data |= read32();
    }

    return data;
}

/* Public */
DataStream::DataStream()
{
    d_ptr = new DataStreamPrivate();
}

DataStream::~DataStream()
{
    delete d_ptr;
}

void DataStream::setData(const std::vector<uint8_t>& data)
{
    d_ptr->m_data = data;
    d_ptr->m_index = 0;
}

void DataStream::setEndian(const std::endian& endian)
{
    d_ptr->m_endian = endian;
}

// Read methods
void DataStream::read(std::vector<uint8_t>& data, const size_t size)
{
    if (d_ptr->m_index + size > d_ptr->m_data.size()) {
        std::cerr << "Out of range" << std::endl;
        return;
    }

    data.resize(size);
    std::memcpy(data.data(), d_ptr->m_data.data() + d_ptr->m_index, size);
    d_ptr->m_index += size;
}

DataStream& DataStream::operator>>(int8_t& data)
{
    data = d_ptr->read8();
    return *this;
}

DataStream& DataStream::operator>>(int16_t& data)
{
    data = d_ptr->read16();
    return *this;
}

DataStream& DataStream::operator>>(int32_t& data)
{
    data = d_ptr->read32();
    return *this;
}

DataStream& DataStream::operator>>(int64_t& data)
{
    data = d_ptr->read64();
    return *this;
}

DataStream& DataStream::operator>>(uint8_t& data)
{
    return *this >> reinterpret_cast<int8_t&>(data);
}

DataStream& DataStream::operator>>(uint16_t& data)
{
    return *this >> reinterpret_cast<int16_t&>(data);
}

DataStream& DataStream::operator>>(uint32_t& data)
{
    return *this >> reinterpret_cast<int32_t&>(data);
}

DataStream& DataStream::operator>>(uint64_t& data)
{
    return *this >> reinterpret_cast<int64_t&>(data);
}

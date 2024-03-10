#include "fvr/file.h"

/* FilePrivate class */
class File::FilePrivate
{
    friend class File;

public:
    // Base file functions
    bool open(const std::string &file, const std::ios_base::openmode &mode);
    void close();
    bool isOpen();

    // Base read functions
    uint8_t readUInt8();
    int8_t readInt8();
    float readFloat();
    double readDouble();
    void read(void *buffer, const std::streamsize &size);

    // Big endian read functions
    uint16_t readUInt16();
    uint32_t readUInt32();
    uint64_t readUInt64();

    int16_t readInt16();
    int32_t readInt32();
    int64_t readInt64();

    // Base write functions
    void writeUInt8(const uint8_t &value);
    void writeInt8(const int8_t &value);
    void writeFloat(const float &value);
    void writeDouble(const double &value);
    void write(const void *buffer, const std::streamsize &size);

    // Big endian write functions
    void writeUInt16(const uint16_t &value);
    void writeUInt32(const uint32_t &value);
    void writeUInt64(const uint64_t &value);

    void writeInt16(const int16_t &value);
    void writeInt32(const int32_t &value);
    void writeInt64(const int64_t &value);

private:
    std::fstream m_file;
    std::endian m_endian;
};

// Base file functions
bool File::FilePrivate::open(const std::string &file, const std::ios_base::openmode &mode)
{
    if (isOpen())
    {
        return false;
    }

    m_file.open(file, mode);

    return isOpen();
}

void File::FilePrivate::close()
{
    m_file.close();
}

bool File::FilePrivate::isOpen()
{
    return m_file.is_open();
}

// Base read functions
uint8_t File::FilePrivate::readUInt8()
{
    uint8_t value = 0;
    m_file.read(reinterpret_cast<char *>(&value), sizeof(value));
    return value;
}

int8_t File::FilePrivate::readInt8()
{
    int8_t value = 0;
    m_file.read(reinterpret_cast<char *>(&value), sizeof(value));
    return value;
}

float File::FilePrivate::readFloat()
{
    uint32_t value = readUInt32();
    return *reinterpret_cast<float *>(&value);
}

double File::FilePrivate::readDouble()
{
    uint64_t value = readUInt64();
    return *reinterpret_cast<double *>(&value);
}

void File::FilePrivate::read(void *buffer, const std::streamsize &size)
{
    m_file.read((char *)(buffer), size);
}

uint16_t File::FilePrivate::readUInt16()
{
    if (!isOpen())
    {
        return 0;
    }

    uint8_t value1 = readUInt8();
    uint8_t value2 = readUInt8();

    if (m_endian == std::endian::big)
    {
        return (value1 << 8) | value2;
    }
    else
    {
        return (value2 << 8) | value1;
    }
}

uint32_t File::FilePrivate::readUInt32()
{
    if (!isOpen())
    {
        return 0;
    }

    uint16_t value1 = readUInt16();
    uint16_t value2 = readUInt16();

    if (m_endian == std::endian::big)
    {
        return (value1 << 16) | value2;
    }
    else
    {
        return (value2 << 16) | value1;
    }
}

uint64_t File::FilePrivate::readUInt64()
{
    if (!isOpen())
    {
        return 0;
    }

    uint64_t value1 = readUInt32();
    uint64_t value2 = readUInt32();

    if (m_endian == std::endian::big)
    {
        return (value1 << 32) | value2;
    }
    else
    {
        return (value2 << 32) | value1;
    }
}

int16_t File::FilePrivate::readInt16()
{
    return static_cast<int16_t>(readUInt16());
}

int32_t File::FilePrivate::readInt32()
{
    return static_cast<int32_t>(readUInt32());
}

int64_t File::FilePrivate::readInt64()
{
    return static_cast<int64_t>(readUInt64());
}

// Base write functions
void File::FilePrivate::writeUInt8(const uint8_t &value)
{
    m_file.write((const char *)(&value), sizeof(value));
}

void File::FilePrivate::writeInt8(const int8_t &value)
{
    m_file.write((const char *)(&value), sizeof(value));
}

void File::FilePrivate::writeFloat(const float &value)
{
    float tmpFloat = value;
    uint32_t tmp = *reinterpret_cast<uint32_t *>(&tmpFloat);
    writeUInt32(tmp);
}

void File::FilePrivate::writeDouble(const double &value)
{
    double tmpFloat = value;
    uint64_t tmp = *reinterpret_cast<uint64_t *>(&tmpFloat);
    writeUInt64(tmp);
}

void File::FilePrivate::write(const void *buffer, const std::streamsize &size)
{
    m_file.write((const char *)(buffer), size);
}

void File::FilePrivate::writeUInt16(const uint16_t &value)
{
    if (m_endian == std::endian::big)
    {
        writeUInt8((value >> 8) & 0xFF);
        writeUInt8(value & 0xFF);
    }
    else
    {
        writeUInt8(value & 0xFF);
        writeUInt8((value >> 8) & 0xFF);
    }
}

void File::FilePrivate::writeUInt32(const uint32_t &value)
{
    if (m_endian == std::endian::big)
    {
        writeUInt16((value >> 16) & 0xFFFF);
        writeUInt16(value & 0xFFFF);
    }
    else
    {
        writeUInt16(value & 0xFFFF);
        writeUInt16((value >> 16) & 0xFFFF);
    }
}

void File::FilePrivate::writeUInt64(const uint64_t &value)
{
    if (m_endian == std::endian::big)
    {
        writeUInt32((value >> 32) & 0xFFFFFFFF);
        writeUInt32(value & 0xFFFFFFFF);
    }
    else
    {
        writeUInt32(value & 0xFFFFFFFF);
        writeUInt32((value >> 32) & 0xFFFFFFFF);
    }
}

void File::FilePrivate::writeInt16(const int16_t &value)
{
    writeUInt16(static_cast<uint16_t>(value));
}

void File::FilePrivate::writeInt32(const int32_t &value)
{
    writeUInt32(static_cast<uint32_t>(value));
}

void File::FilePrivate::writeInt64(const int64_t &value)
{
    writeUInt64(static_cast<uint64_t>(value));
}

/* File class */
File::File()
{
    d_ptr = new FilePrivate();

    d_ptr->m_endian = std::endian::native;
}

File::~File()
{
    if (isOpen())
    {
        close();
    }

    delete d_ptr;
}

// Base file functions
bool File::open(const std::string &filename, const std::ios_base::openmode &mode)
{
    return d_ptr->open(filename, mode);
}

void File::close()
{
    d_ptr->close();
}

bool File::isOpen()
{
    return d_ptr->isOpen();
}

void File::setEndian(const std::endian &endian)
{
    d_ptr->m_endian = endian;
}

std::endian File::getEndian()
{
    return d_ptr->m_endian;
}

void File::seek(const std::streampos &pos)
{
    d_ptr->m_file.seekp(pos);
}

std::streampos File::tell()
{
    return d_ptr->m_file.tellp();
}

bool File::atEnd()
{
    return d_ptr->m_file.eof() || d_ptr->m_file.peek() == EOF;
}

void File::read(void *buffer, const std::streamsize &size)
{
    d_ptr->read(buffer, size);
}

void File::write(const void *buffer, const std::streamsize &size)
{
    d_ptr->write(buffer, size);
}

// Operator overloads
// Read
File &File::operator>>(uint8_t &value)
{
    value = d_ptr->readUInt8();

    return *this;
}

File &File::operator>>(int8_t &value)
{
    value = d_ptr->readInt8();

    return *this;
}

File &File::operator>>(uint16_t &value)
{
    value = d_ptr->readUInt16();

    return *this;
}

File &File::operator>>(uint32_t &value)
{
    value = d_ptr->readUInt32();

    return *this;
}

File &File::operator>>(uint64_t &value)
{
    value = d_ptr->readUInt64();

    return *this;
}

File &File::operator>>(int16_t &value)
{
    value = d_ptr->readInt16();

    return *this;
}

File &File::operator>>(int32_t &value)
{
    value = d_ptr->readInt32();

    return *this;
}

File &File::operator>>(int64_t &value)
{
    value = d_ptr->readInt64();

    return *this;
}

File &File::operator>>(float &value)
{
    value = d_ptr->readFloat();

    return *this;
}

File &File::operator>>(double &value)
{
    value = d_ptr->readDouble();

    return *this;
}

// Write
File &File::operator<<(const uint8_t &value)
{
    d_ptr->writeUInt8(value);

    return *this;
}

File &File::operator<<(const int8_t &value)
{
    d_ptr->writeInt8(value);

    return *this;
}

File &File::operator<<(const uint16_t &value)
{
    d_ptr->writeUInt16(value);

    return *this;
}

File &File::operator<<(const uint32_t &value)
{
    d_ptr->writeUInt32(value);

    return *this;
}

File &File::operator<<(const uint64_t &value)
{
    d_ptr->writeUInt64(value);

    return *this;
}

File &File::operator<<(const int16_t &value)
{
    d_ptr->writeInt16(value);

    return *this;
}

File &File::operator<<(const int32_t &value)
{
    d_ptr->writeInt32(value);

    return *this;
}

File &File::operator<<(const int64_t &value)
{
    d_ptr->writeInt64(value);

    return *this;
}

File &File::operator<<(const float &value)
{
    d_ptr->writeFloat(value);

    return *this;
}

File &File::operator<<(const double &value)
{
    d_ptr->writeDouble(value);

    return *this;
}

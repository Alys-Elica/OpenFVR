#include "file.h"

/* FilePrivate class */
class FilePrivate
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
    std::string readString();
    void read(void *buffer, const std::streamsize &size);

    // Big endian read functions
    uint16_t readUInt16BE();
    uint32_t readUInt32BE();
    uint64_t readUInt64BE();

    int16_t readInt16BE();
    int32_t readInt32BE();
    int64_t readInt64BE();

    // Little endian read functions
    uint16_t readUInt16LE();
    uint32_t readUInt32LE();
    uint64_t readUInt64LE();

    int16_t readInt16LE();
    int32_t readInt32LE();
    int64_t readInt64LE();

    // Base write functions
    void writeUInt8(const uint8_t &value);
    void writeInt8(const int8_t &value);
    void writeFloat(const float &value);
    void writeDouble(const double &value);
    void writeString(const std::string &value);
    void write(const void *buffer, const std::streamsize &size);

    // Big endian write functions
    void writeUInt16BE(const uint16_t &value);
    void writeUInt32BE(const uint32_t &value);
    void writeUInt64BE(const uint64_t &value);

    void writeInt16BE(const int16_t &value);
    void writeInt32BE(const int32_t &value);
    void writeInt64BE(const int64_t &value);

    // Little endian write functions
    void writeUInt16LE(const uint16_t &value);
    void writeUInt32LE(const uint32_t &value);
    void writeUInt64LE(const uint64_t &value);

    void writeInt16LE(const int16_t &value);
    void writeInt32LE(const int32_t &value);
    void writeInt64LE(const int64_t &value);

private:
    std::fstream m_file;
    File::Endian m_endian;
};

// Base file functions
bool FilePrivate::open(const std::string &file, const std::ios_base::openmode &mode)
{
    if (isOpen())
    {
        return false;
    }

    m_file.open(file, mode);

    return isOpen();
}

void FilePrivate::close()
{
    m_file.close();
}

bool FilePrivate::isOpen()
{
    return m_file.is_open();
}

// Base read functions
uint8_t FilePrivate::readUInt8()
{
    uint8_t value = 0;
    m_file.read(reinterpret_cast<char *>(&value), sizeof(value));
    return value;
}

int8_t FilePrivate::readInt8()
{
    int8_t value = 0;
    m_file.read(reinterpret_cast<char *>(&value), sizeof(value));
    return value;
}

float FilePrivate::readFloat()
{
    uint32_t value;

    if (m_endian == File::Endian::BigEndian)
    {
        value = readUInt32BE();
    }
    else
    {
        value = readUInt32LE();
    }

    return *reinterpret_cast<float *>(&value);
}

double FilePrivate::readDouble()
{
    uint64_t value;

    if (m_endian == File::Endian::BigEndian)
    {
        value = readUInt64BE();
    }
    else
    {
        value = readUInt64LE();
    }

    return *reinterpret_cast<double *>(&value);
}

std::string FilePrivate::readString()
{
    std::string value;

    uint64_t size;
    if (m_endian == File::Endian::BigEndian)
    {
        size = readUInt64BE();
    }
    else
    {
        size = readUInt64LE();
    }

    value.resize(size);
    m_file.read(value.data(), size);

    return value;
}

void FilePrivate::read(void *buffer, const std::streamsize &size)
{
    m_file.read((char *)(buffer), size);
}

// Big endian read functions
uint16_t FilePrivate::readUInt16BE()
{
    if (!isOpen())
    {
        return 0;
    }

    uint8_t value1 = readUInt8();
    uint8_t value2 = readUInt8();

    return (value1 << 8) | value2;
}

uint32_t FilePrivate::readUInt32BE()
{
    if (!isOpen())
    {
        return 0;
    }

    uint16_t value1 = readUInt16BE();
    uint16_t value2 = readUInt16BE();

    return (value1 << 16) | value2;
}

uint64_t FilePrivate::readUInt64BE()
{
    if (!isOpen())
    {
        return 0;
    }

    uint64_t value1 = readUInt32BE();
    uint64_t value2 = readUInt32BE();

    return (value1 << 32) | value2;
}

int16_t FilePrivate::readInt16BE()
{
    return static_cast<int16_t>(readUInt16BE());
}

int32_t FilePrivate::readInt32BE()
{
    return static_cast<int32_t>(readUInt32BE());
}

int64_t FilePrivate::readInt64BE()
{
    return static_cast<int64_t>(readUInt64BE());
}

// Little endian read functions
uint16_t FilePrivate::readUInt16LE()
{
    if (!isOpen())
    {
        return 0;
    }

    uint8_t value1 = readUInt8();
    uint8_t value2 = readUInt8();

    return (value2 << 8) | value1;
}

uint32_t FilePrivate::readUInt32LE()
{
    if (!isOpen())
    {
        return 0;
    }

    uint16_t value1 = readUInt16LE();
    uint16_t value2 = readUInt16LE();

    return (value2 << 16) | value1;
}

uint64_t FilePrivate::readUInt64LE()
{
    if (!isOpen())
    {
        return 0;
    }

    uint64_t value1 = readUInt32LE();
    uint64_t value2 = readUInt32LE();

    return (value2 << 32) | value1;
}

int16_t FilePrivate::readInt16LE()
{
    return static_cast<int16_t>(readUInt16LE());
}

int32_t FilePrivate::readInt32LE()
{
    return static_cast<int32_t>(readUInt32LE());
}

int64_t FilePrivate::readInt64LE()
{
    return static_cast<int64_t>(readUInt64LE());
}

// Base write functions
void FilePrivate::writeUInt8(const uint8_t &value)
{
    m_file.write((const char *)(&value), sizeof(value));
}

void FilePrivate::writeInt8(const int8_t &value)
{
    m_file.write((const char *)(&value), sizeof(value));
}

void FilePrivate::writeFloat(const float &value)
{
    float tmpFloat = value;
    uint32_t tmp = *reinterpret_cast<uint32_t *>(&tmpFloat);
    if (m_endian == File::Endian::BigEndian)
    {
        writeUInt32BE(tmp);
    }
    else
    {
        writeUInt32LE(tmp);
    }
}

void FilePrivate::writeDouble(const double &value)
{
    double tmpFloat = value;
    uint64_t tmp = *reinterpret_cast<uint64_t *>(&tmpFloat);
    if (m_endian == File::Endian::BigEndian)
    {
        writeUInt64BE(tmp);
    }
    else
    {
        writeUInt64LE(tmp);
    }
}

void FilePrivate::writeString(const std::string &value)
{
    if (m_endian == File::Endian::BigEndian)
    {
        writeUInt64BE(value.size());
    }
    else
    {
        writeUInt64LE(value.size());
    }

    m_file.write(value.data(), value.size());
}

void FilePrivate::write(const void *buffer, const std::streamsize &size)
{
    m_file.write((const char *)(buffer), size);
}

// Big endian write functions
void FilePrivate::writeUInt16BE(const uint16_t &value)
{
    writeUInt8((value >> 8) & 0xFF);
    writeUInt8(value & 0xFF);
}

void FilePrivate::writeUInt32BE(const uint32_t &value)
{
    writeUInt16BE((value >> 16) & 0xFFFF);
    writeUInt16BE(value & 0xFFFF);
}

void FilePrivate::writeUInt64BE(const uint64_t &value)
{
    writeUInt32BE((value >> 32) & 0xFFFFFFFF);
    writeUInt32BE(value & 0xFFFFFFFF);
}

void FilePrivate::writeInt16BE(const int16_t &value)
{
    writeUInt16BE(static_cast<uint16_t>(value));
}

void FilePrivate::writeInt32BE(const int32_t &value)
{
    writeUInt32BE(static_cast<uint32_t>(value));
}

void FilePrivate::writeInt64BE(const int64_t &value)
{
    writeUInt64BE(static_cast<uint64_t>(value));
}

// Little endian write functions
void FilePrivate::writeUInt16LE(const uint16_t &value)
{
    writeUInt8(value & 0xFF);
    writeUInt8((value >> 8) & 0xFF);
}

void FilePrivate::writeUInt32LE(const uint32_t &value)
{
    writeUInt16LE(value & 0xFFFF);
    writeUInt16LE((value >> 16) & 0xFFFF);
}

void FilePrivate::writeUInt64LE(const uint64_t &value)
{
    writeUInt32LE(value & 0xFFFFFFFF);
    writeUInt32LE((value >> 32) & 0xFFFFFFFF);
}

void FilePrivate::writeInt16LE(const int16_t &value)
{
    writeUInt16LE(static_cast<uint16_t>(value));
}

void FilePrivate::writeInt32LE(const int32_t &value)
{
    writeUInt32LE(static_cast<uint32_t>(value));
}

void FilePrivate::writeInt64LE(const int64_t &value)
{
    writeUInt64LE(static_cast<uint64_t>(value));
}

/* File class */
File::File()
{
    d_ptr = new FilePrivate();

    d_ptr->m_endian = File::Endian::BigEndian;
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

void File::setEndian(const Endian &endian)
{
    d_ptr->m_endian = endian;
}

File::Endian File::getEndian()
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
    return d_ptr->m_file.eof();
}

// Read functions
uint8_t File::readUInt8()
{
    return d_ptr->readUInt8();
}

int8_t File::readInt8()
{
    return d_ptr->readInt8();
}

void File::read(void *buffer, const std::streamsize &size)
{
    d_ptr->read(buffer, size);
}

uint16_t File::readUInt16()
{
    if (d_ptr->m_endian == Endian::BigEndian)
    {
        return d_ptr->readUInt16BE();
    }
    else
    {
        return d_ptr->readUInt16LE();
    }
}

uint32_t File::readUInt32()
{
    if (d_ptr->m_endian == Endian::BigEndian)
    {
        return d_ptr->readUInt32BE();
    }
    else
    {
        return d_ptr->readUInt32LE();
    }
}

uint64_t File::readUInt64()
{
    if (d_ptr->m_endian == Endian::BigEndian)
    {
        return d_ptr->readUInt64BE();
    }
    else
    {
        return d_ptr->readUInt64LE();
    }
}

int16_t File::readInt16()
{
    if (d_ptr->m_endian == Endian::BigEndian)
    {
        return d_ptr->readInt16BE();
    }
    else
    {
        return d_ptr->readInt16LE();
    }
}

int32_t File::readInt32()
{
    if (d_ptr->m_endian == Endian::BigEndian)
    {
        return d_ptr->readInt32BE();
    }
    else
    {
        return d_ptr->readInt32LE();
    }
}

int64_t File::readInt64()
{
    if (d_ptr->m_endian == Endian::BigEndian)
    {
        return d_ptr->readInt64BE();
    }
    else
    {
        return d_ptr->readInt64LE();
    }
}

float File::readFloat()
{
    return d_ptr->readFloat();
}

double File::readDouble()
{
    return d_ptr->readDouble();
}

std::string File::readString()
{
    return d_ptr->readString();
}

// Write functions
void File::writeUInt8(const uint8_t &value)
{
    d_ptr->writeUInt8(value);
}

void File::writeInt8(const int8_t &value)
{
    d_ptr->writeInt8(value);
}

void File::write(const void *buffer, const std::streamsize &size)
{
    d_ptr->write(buffer, size);
}

void File::writeUInt16(const uint16_t &value)
{
    if (d_ptr->m_endian == Endian::BigEndian)
    {
        d_ptr->writeUInt16BE(value);
    }
    else
    {
        d_ptr->writeUInt16LE(value);
    }
}

void File::writeUInt32(const uint32_t &value)
{
    if (d_ptr->m_endian == Endian::BigEndian)
    {
        d_ptr->writeUInt32BE(value);
    }
    else
    {
        d_ptr->writeUInt32LE(value);
    }
}

void File::writeUInt64(const uint64_t &value)
{
    if (d_ptr->m_endian == Endian::BigEndian)
    {
        d_ptr->writeUInt64BE(value);
    }
    else
    {
        d_ptr->writeUInt64LE(value);
    }
}

void File::writeInt16(const int16_t &value)
{
    if (d_ptr->m_endian == Endian::BigEndian)
    {
        d_ptr->writeInt16BE(value);
    }
    else
    {
        d_ptr->writeInt16LE(value);
    }
}

void File::writeInt32(const int32_t &value)
{
    if (d_ptr->m_endian == Endian::BigEndian)
    {
        d_ptr->writeInt32BE(value);
    }
    else
    {
        d_ptr->writeInt32LE(value);
    }
}

void File::writeInt64(const int64_t &value)
{
    if (d_ptr->m_endian == Endian::BigEndian)
    {
        d_ptr->writeInt64BE(value);
    }
    else
    {
        d_ptr->writeInt64LE(value);
    }
}

void File::writeFloat(const float &value)
{
    d_ptr->writeFloat(value);
}

void File::writeDouble(const double &value)
{
    d_ptr->writeDouble(value);
}

void File::writeString(const std::string &string)
{
    d_ptr->writeString(string);
}

// Operator overloads
// Read
File &File::operator>>(uint8_t &value)
{
    value = readUInt8();

    return *this;
}

File &File::operator>>(int8_t &value)
{
    value = readInt8();

    return *this;
}

File &File::operator>>(uint16_t &value)
{
    value = readUInt16();

    return *this;
}

File &File::operator>>(uint32_t &value)
{
    value = readUInt32();

    return *this;
}

File &File::operator>>(uint64_t &value)
{
    value = readUInt64();

    return *this;
}

File &File::operator>>(int16_t &value)
{
    value = readInt16();

    return *this;
}

File &File::operator>>(int32_t &value)
{
    value = readInt32();

    return *this;
}

File &File::operator>>(int64_t &value)
{
    value = readInt64();

    return *this;
}

File &File::operator>>(float &value)
{
    value = readFloat();

    return *this;
}

File &File::operator>>(double &value)
{
    value = readDouble();

    return *this;
}

File &File::operator>>(std::string &value)
{
    value = readString();

    return *this;
}

// Write
File &File::operator<<(const uint8_t &value)
{
    writeUInt8(value);

    return *this;
}

File &File::operator<<(const int8_t &value)
{
    writeInt8(value);

    return *this;
}

File &File::operator<<(const uint16_t &value)
{
    writeUInt16(value);

    return *this;
}

File &File::operator<<(const uint32_t &value)
{
    writeUInt32(value);

    return *this;
}

File &File::operator<<(const uint64_t &value)
{
    writeUInt64(value);

    return *this;
}

File &File::operator<<(const int16_t &value)
{
    writeInt16(value);

    return *this;
}

File &File::operator<<(const int32_t &value)
{
    writeInt32(value);

    return *this;
}

File &File::operator<<(const int64_t &value)
{
    writeInt64(value);

    return *this;
}

File &File::operator<<(const float &value)
{
    writeFloat(value);

    return *this;
}

File &File::operator<<(const double &value)
{
    writeDouble(value);

    return *this;
}

File &File::operator<<(const std::string &value)
{
    writeString(value);

    return *this;
}

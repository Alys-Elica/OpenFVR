#ifndef FILE_H
#define FILE_H

#include "libfvr_globals.h"

#include <bit>
#include <cstdint>
#include <fstream>
#include <string>

class FilePrivate;
class LIBFVR_EXPORT File final
{
public:
    File();
    ~File();

    File(const File &other) = delete;
    File &operator=(const File &other) = delete;

    // Base file functions
    bool open(const std::string &file, const std::ios_base::openmode &mode);
    void close();
    bool isOpen();

    void setEndian(const std::endian &endian);
    std::endian getEndian();

    void seek(const std::streampos &pos);
    std::streampos tell();
    bool atEnd();

    // Read functions
    uint8_t readUInt8();
    int8_t readInt8();
    void read(void *buffer, const std::streamsize &size);

    uint16_t readUInt16();
    uint32_t readUInt32();
    uint64_t readUInt64();

    int16_t readInt16();
    int32_t readInt32();
    int64_t readInt64();

    float readFloat();
    double readDouble();

    std::string readString();

    // Write functions
    void writeUInt8(const uint8_t &value);
    void writeInt8(const int8_t &value);
    void write(const void *buffer, const std::streamsize &size);

    void writeUInt16(const uint16_t &value);
    void writeUInt32(const uint32_t &value);
    void writeUInt64(const uint64_t &value);

    void writeInt16(const int16_t &value);
    void writeInt32(const int32_t &value);
    void writeInt64(const int64_t &value);

    void writeFloat(const float &value);
    void writeDouble(const double &value);

    void writeString(const std::string &value);

    // Operator overloads
    // Read
    File &operator>>(uint8_t &value);
    File &operator>>(int8_t &value);

    File &operator>>(uint16_t &value);
    File &operator>>(uint32_t &value);
    File &operator>>(uint64_t &value);

    File &operator>>(int16_t &value);
    File &operator>>(int32_t &value);
    File &operator>>(int64_t &value);

    File &operator>>(float &value);
    File &operator>>(double &value);

    File &operator>>(std::string &value);

    // Write
    File &operator<<(const uint8_t &value);
    File &operator<<(const int8_t &value);

    File &operator<<(const uint16_t &value);
    File &operator<<(const uint32_t &value);
    File &operator<<(const uint64_t &value);

    File &operator<<(const int16_t &value);
    File &operator<<(const int32_t &value);
    File &operator<<(const int64_t &value);

    File &operator<<(const float &value);
    File &operator<<(const double &value);

    File &operator<<(const std::string &value);

private:
    FilePrivate *d_ptr;
};

#endif // FILE_H

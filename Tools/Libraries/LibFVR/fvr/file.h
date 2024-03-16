#ifndef FILE_H
#define FILE_H

#include "libfvr_globals.h"

#include <bit>
#include <cstdint>
#include <fstream>
#include <string>

class LIBFVR_EXPORT File final {
public:
    File();
    ~File();

    File(const File& other) = delete;
    File& operator=(const File& other) = delete;

    // Base file functions
    bool open(const std::string& file, const std::ios_base::openmode& mode);
    void close();
    bool isOpen();

    void setEndian(const std::endian& endian);
    std::endian getEndian();

    void seek(const std::streampos& pos);
    std::streampos tell();
    bool atEnd();

    // Read functions
    void read(void* buffer, const std::streamsize& size);
    void write(const void* buffer, const std::streamsize& size);

    // Operator overloads
    // Read
    File& operator>>(uint8_t& value);
    File& operator>>(int8_t& value);

    File& operator>>(uint16_t& value);
    File& operator>>(uint32_t& value);
    File& operator>>(uint64_t& value);

    File& operator>>(int16_t& value);
    File& operator>>(int32_t& value);
    File& operator>>(int64_t& value);

    File& operator>>(float& value);
    File& operator>>(double& value);

    // Write
    File& operator<<(const uint8_t& value);
    File& operator<<(const int8_t& value);

    File& operator<<(const uint16_t& value);
    File& operator<<(const uint32_t& value);
    File& operator<<(const uint64_t& value);

    File& operator<<(const int16_t& value);
    File& operator<<(const int32_t& value);
    File& operator<<(const int64_t& value);

    File& operator<<(const float& value);
    File& operator<<(const double& value);

private:
    class FilePrivate;
    FilePrivate* d_ptr;
};

#endif // FILE_H

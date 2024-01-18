#ifndef FVR_PAK_H
#define FVR_PAK_H

#include "libfvr_globals.h"

#include <string>
#include <vector>

class FvrPakPrivate;
class LIBFVR_EXPORT FvrPak final
{
public:
    FvrPak();
    ~FvrPak();

    FvrPak(const FvrPak &other) = delete;
    FvrPak &operator=(const FvrPak &other) = delete;

    bool open(const std::string &pakFileName);
    void close();
    bool isOpen() const;

    int fileCount() const;
    std::string fileName(int index) const;
    std::vector<uint8_t> fileData(int index) const;

private:
    FvrPakPrivate *d_ptr;
};

#endif // FVR_PAK_H

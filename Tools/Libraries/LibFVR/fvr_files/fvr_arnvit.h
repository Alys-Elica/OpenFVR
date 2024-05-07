#ifndef FVR_ARNVIT_H
#define FVR_ARNVIT_H

#include "libfvr_globals.h"

#include <cstdint>
#include <string>
#include <vector>

class LIBFVR_EXPORT FvrArnVit final {
public:
    struct ArnVitFile {
        std::string fileName;
        uint32_t width;
        uint32_t height;
        uint32_t fileSize;
        uint32_t offset;

        uint32_t unkn1;
        uint32_t unkn2;
        uint32_t unkn3;
        uint32_t unkn4;

        std::vector<uint8_t> data;
    };

public:
    FvrArnVit();
    ~FvrArnVit();

    FvrArnVit(const FvrArnVit& other) = delete;
    FvrArnVit& operator=(const FvrArnVit& other) = delete;

    bool open(const std::string& vitFileName, const std::string& arnFileName);
    void close();
    bool isOpen() const;

    int fileCount() const;
    ArnVitFile getFile(const int index) const;
    ArnVitFile getFile(const std::string& name) const;
    bool writeToBmp(const int index, const std::string& outputDirectory) const;

private:
    class FvrArnVitPrivate;
    FvrArnVitPrivate* d_ptr;
};

#endif // FVR_ARNVIT_H

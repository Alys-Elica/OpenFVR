#ifndef PAKTOOLS_H
#define PAKTOOLS_H

#include <string>
#include <vector>

struct PakSubFile
{
    std::string fileName;
    std::vector<uint8_t> data;
};

class PakTools
{
public:
    static bool unpack(const std::string &fileName, std::vector<PakSubFile> &uncompressedFiles);

private:
    static void uncompressPakData3(const std::vector<uint8_t> &dataIn, std::vector<uint8_t> &dataOut);
};

#endif // PAKTOOLS_H

#ifndef CONVERTERPACK_H
#define CONVERTERPACK_H

#include <string>
#include <vector>

namespace ConverterPack
{
    bool unpack(const std::string &fileName, const std::string &outputFolder);
    //bool pack(const std::vector<std::string> &fileNames, const std::string outputName);
};

#endif // CONVERTERPACK_H

#ifndef PAKTOOLS_H
#define PAKTOOLS_H

#include <string>
#include <vector>

namespace ConverterPack
{
    bool unpack(const std::string &fileName, const std::string &outputFolder);
    //bool pack(const std::vector<std::string> &fileNames, const std::string outputName);
};

#endif // PAKTOOLS_H

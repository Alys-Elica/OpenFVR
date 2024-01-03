#include <fstream>
#include <iostream>

#include "paktools.h"

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <pak file> [pak file] ..." << std::endl;
        return false;
    }

    for (int i = 1; i < argc; i++)
    {
        std::string pakFileName = argv[i];

        std::vector<PakSubFile> listfile;
        if (PakTools::unpack(pakFileName, listfile))
        {
            std::cout << "Unpacked " << listfile.size() << " files from " << pakFileName << std::endl;
            for (const PakSubFile &subFile : listfile)
            {
                std::cout << "  Saving file " << subFile.fileName << std::endl;
                // Write file
                std::string outFileName = subFile.fileName;
                std::ofstream outFile(outFileName, std::ios_base::out | std::ios_base::binary);
                outFile.write(reinterpret_cast<const char *>(subFile.data.data()), subFile.data.size());
                outFile.close();
            }
        }
        else
        {
            std::cerr << "Unable to unpack " << pakFileName << std::endl;
        }
    }
}

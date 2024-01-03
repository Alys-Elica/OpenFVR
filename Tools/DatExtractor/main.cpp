#include <filesystem>
#include <fstream>
#include <iostream>

#include <fvr_files/fvr_dat.h>

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <dat file> [dat file] ..." << std::endl;
        return false;
    }

    for (int i = 1; i < argc; i++)
    {
        std::string datFileName = argv[i];
        FvrDat dat;

        if (!dat.open(datFileName))
        {
            std::cerr << "Failed to open " << datFileName << std::endl;
            return false;
        }

        // Create output directory
        std::string outputDir = datFileName + "_out/";
        if (!std::filesystem::create_directory(outputDir))
        {
            std::cerr << "Failed to create output directory" << std::endl;
            return false;
        }

        // Extract files
        std::cout << "Extracting " << dat.fileCount() << " files from " << datFileName << std::endl;

        for (int j = 0; j < dat.fileCount(); j++)
        {
            std::cout << "  Saving file " << dat.fileName(j) << std::endl;

            std::string fileName = outputDir + dat.fileName(j);
            std::vector<uint8_t> fileData = dat.fileData(j);

            std::ofstream outFile(fileName, std::ios_base::out | std::ios_base::binary);
            outFile.write(reinterpret_cast<const char *>(fileData.data()), fileData.size());
            outFile.close();
        }
    }
}

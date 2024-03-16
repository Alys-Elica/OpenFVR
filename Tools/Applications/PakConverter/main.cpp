#include <fstream>
#include <iostream>

#include <fvr_files/fvr_pak.h>

int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <pak file> [pak file] ..." << std::endl;
        return false;
    }

    for (int i = 1; i < argc; i++) {
        std::string pakFileName = argv[i];

        FvrPak pak;
        if (!pak.open(pakFileName)) {
            std::cerr << "Unable to open file " << pakFileName << std::endl;
            continue;
        }

        for (int i = 0; i < pak.fileCount(); i++) {
            std::string fileName = pak.fileName(i);
            std::vector<uint8_t> fileData = pak.fileData(i);

            if (fileData.size() > 0) {
                // Write file
                std::ofstream file(fileName, std::ios::binary);
                file.write(reinterpret_cast<const char*>(fileData.data()), fileData.size());
                file.close();
            }
        }

        pak.close();
    }
}

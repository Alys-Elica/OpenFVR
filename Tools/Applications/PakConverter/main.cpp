#include <fstream>
#include <iostream>

#include <ofnx/files/pak.h>
#include <ofnx/tools/log.h>

int main(int argc, char* argv[])
{
    if (argc < 2) {
        LOG_CRITICAL("Usage: {} <pak_file> [pak_file] ...", argv[0]);
        return false;
    }

    for (int i = 1; i < argc; i++) {
        std::string pakFileName = argv[i];

        ofnx::files::Pak pak;
        if (!pak.open(pakFileName)) {
            LOG_CRITICAL("Unable to open file {}", pakFileName);
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

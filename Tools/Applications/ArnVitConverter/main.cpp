#include <filesystem>
#include <fstream>
#include <iostream>

#include <ofnx/files/arnvit.h>
#include <ofnx/tools/log.h>

int main(int argc, char* argv[])
{
    if (argc < 3) {
        LOG_CRITICAL("Usage: {} <vit_file> <arn_file>", argv[0]);
        return false;
    }

    std::string vitFileName = argv[1];
    std::string arnFileName = argv[2];

    ofnx::files::ArnVit arnVit;
    if (!arnVit.open(vitFileName, arnFileName)) {
        LOG_CRITICAL("Unable to open ARN/VIT files");
        return false;
    }

    // Create a directory to store the extracted files
    const std::string outputDir = "extracted_files/";
    std::filesystem::create_directory(outputDir);
    int fileCount = arnVit.fileCount();
    for (int i = 0; i < fileCount; i++) {
        arnVit.writeToBmp(i, outputDir);
    }

    arnVit.close();

    return 0;
}

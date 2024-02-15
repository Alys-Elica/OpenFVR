#include <filesystem>
#include <fstream>
#include <iostream>

#include <fvr_files/fvr_arnvit.h>

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        std::cerr << "Usage: " << argv[0] << " <vit file> <arn file>" << std::endl;
        return false;
    }

    std::string vitFileName = argv[1];
    std::string arnFileName = argv[2];

    FvrArnVit arnVit;
    if (!arnVit.open(vitFileName, arnFileName))
    {
        std::cerr << "Unable to open ARN/VIT files" << std::endl;
        return false;
    }

    // Create a directory to store the extracted files
    const std::string outputDir = "extracted_files/";
    std::filesystem::create_directory(outputDir);
    int fileCount = arnVit.fileCount();
    for (int i = 0; i < fileCount; i++)
    {
        arnVit.writeToBmp(i, outputDir);
    }

    arnVit.close();

    return 0;
}

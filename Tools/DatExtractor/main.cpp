#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

struct File
{
    std::string name;
    uint32_t size;
    uint32_t offset;
};

uint32_t readUInt32(std::fstream &file)
{
    uint8_t data[4];
    file.read((char *)data, 4);

    return (uint32_t)data[0] | ((uint32_t)data[1] << 8) | ((uint32_t)data[2] << 16) | ((uint32_t)data[3] << 24);
}

bool datExtract(const std::string &datFileName)
{
    std::cout << "Converting " << datFileName << std::endl;

    std::fstream fileIn(datFileName, std::ios_base::in | std::ios_base::binary);

    if (!fileIn.is_open())
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

    // Check magic
    fileIn.seekp(0);
    std::string magic(4, '\0');
    fileIn.read(magic.data(), 4);
    if (magic != "BIGF")
    {
        std::cerr << "Invalid magic" << std::endl;
        return false;
    }

    // Read file index
    std::vector<File> listFile;

    fileIn.seekp(0x50);
    char current;
    fileIn.read(&current, 1);
    do
    {
        File file;
        while (current != '\0')
        {
            file.name += current;
            fileIn.read(&current, 1);
        }

        file.size = readUInt32(fileIn);
        file.offset = readUInt32(fileIn);

        listFile.push_back(file);

        fileIn.read(&current, 1);
    } while (current != '\0');

    // Extract files
    for (const File &file : listFile)
    {
        std::cout << "Extracting " << file.name << std::endl;

        std::fstream fileOut(outputDir + file.name, std::ios_base::out | std::ios_base::binary);

        if (!fileOut.is_open())
        {
            std::cerr << "Failed to open " << file.name << std::endl;
            return false;
        }

        std::vector<char> data(file.size);
        fileIn.read(data.data(), file.size);
        fileOut.write(data.data(), file.size);

        fileOut.close();
    }

    return true;
}

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

        datExtract(datFileName);
    }
}

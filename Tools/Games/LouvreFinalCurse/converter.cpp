#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include <fvr_files/fvr_arnvit.h>
#include <fvr_files/fvr_pak.h>
#include <fvr_files/fvr_script.h>

#ifdef _WIN32
#include <windows.h>
#endif

void toLower(std::string& str)
{
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
}

void copyFile(const std::string& path, const std::string& pathOut, const std::string& extension)
{
    std::string ext = std::filesystem::path(path).extension().string();
    std::string name = std::filesystem::path(path).filename().string();
    toLower(ext);
    toLower(name);
    if (ext == extension) {
        try {
            std::filesystem::copy(path, pathOut + name);
        } catch (const std::exception&) {
            // std::cerr << e.what() << '\n';
        }
    }
}

void copyFiles(const std::string& path, const std::string& pathOut, const std::string& extension)
{
    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        std::string path = entry.path().string();
        copyFile(path, pathOut, extension);
    }
}

void copyImage(const std::string& path, const std::string& pathOut)
{
    copyFiles(path, pathOut, ".gif");
    copyFiles(path, pathOut, ".pcx");
}

std::vector<uint8_t> readScript(const std::string& fileIn)
{
    FvrPak fvrPak;
    if (!fvrPak.open(fileIn)) {
        std::cerr << "Error opening PAK file: " << fileIn << std::endl;
        return {};
    }

    if (fvrPak.fileCount() != 1) {
        std::cerr << "Error opening PAK file: " << fileIn << std::endl;
        return {};
    }

    return fvrPak.fileData(0);
}

void saveScript(const std::vector<uint8_t>& data, const std::string& fileOut)
{
    const std::string tmpFileName = "tmp.lst";
    std::ofstream fileTmp(tmpFileName, std::ios::binary);
    fileTmp.write(reinterpret_cast<const char*>(data.data()), data.size());
    fileTmp.close();

    FvrScript fvrScript;
    if (!fvrScript.parseLst(tmpFileName)) {
        std::cerr << "Error parsing script: " << tmpFileName << std::endl;
        return;
    }

    if (!fvrScript.saveLst(fileOut)) {
        std::cerr << "Error saving script: " << fileOut << std::endl;
        return;
    }

    std::filesystem::remove(tmpFileName);
}

void copyVideo(const std::string& path, const std::string& pathOut)
{
    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        std::string path = entry.path().string();
        std::string ext = std::filesystem::path(path).extension().string();
        std::string name = std::filesystem::path(path).stem().string();
        toLower(ext);
        toLower(name);
        if (ext == ".4xm") {
            try {
                std::filesystem::copy(path, pathOut + name + ".4xm");
            } catch (const std::exception&) {
                // std::cerr << e.what() << '\n';
            }
        }
    }
}

int main(int argc, char* argv[])
{
    std::string pathIn = "input/";
    std::string pathOut = "data/";

    std::string pathInstall = pathIn + "CD1/Install/";
    std::string pathData1 = pathIn + "CD1/Data/";
    std::string pathData2 = pathIn + "CD2/Data/";

    std::string pathOutAudio = pathOut + "audio/";
    std::string pathOutImage = pathOut + "image/";
    std::string pathOutVideo = pathOut + "video/";
    std::string pathOutWarp = pathOut + "warp/";
    std::string pathOutTest = pathOut + "tst/";
    std::string pathOutScript = pathOut + "script/";

    // Create output directories
    std::filesystem::create_directories(pathOut);
    std::filesystem::create_directories(pathOutAudio);
    std::filesystem::create_directories(pathOutImage);
    // std::filesystem::create_directories(pathOutImage + "objects/");
    std::filesystem::create_directories(pathOutVideo);
    std::filesystem::create_directories(pathOutWarp);
    std::filesystem::create_directories(pathOutTest);
    std::filesystem::create_directories(pathOutScript);

    // Sounds
    copyFiles(pathInstall, pathOutAudio, ".wav");
    copyFiles(pathData1, pathOutAudio, ".wav");
    copyFiles(pathData2, pathOutAudio, ".wav");

    // Images
    copyImage(pathInstall, pathOutImage);
    copyImage(pathData1, pathOutImage);
    copyImage(pathData2, pathOutImage);

    // Videos
    copyVideo(pathInstall, pathOutVideo);
    copyVideo(pathData1, pathOutVideo);
    copyVideo(pathData2, pathOutVideo);

    // ARN/VIT
    copyFile(pathInstall + "BData1.arn", pathOut, ".arn");
    copyFile(pathInstall + "BDataHeader.vit", pathOut, ".vit");

    // Texts
    copyFile(pathInstall + "Textes.txt", pathOut, ".txt");

    // Scripts
    std::vector<uint8_t> dataScript1 = readScript(pathData1 + "Script.pak");
    // Patching script 1
    dataScript1[375'198] = ';';
    saveScript(dataScript1, pathOutScript + "script_1.lst");

    std::vector<uint8_t> dataScript2 = readScript(pathData2 + "Script2.pak");
    // Patching script 2
    dataScript2[10'846] = '0';
    dataScript2[423'083] = ';';
    dataScript2[423'588] = ';';
    dataScript2[454'934] = '\n';
    dataScript2[513'709] = '\n';
    dataScript2[671'233] = ';';
    saveScript(dataScript2, pathOutScript + "script_2.lst");

    // VR
    copyFiles(pathInstall, pathOutWarp, ".vr");
    copyFiles(pathData1, pathOutWarp, ".vr");
    copyFiles(pathData2, pathOutWarp, ".vr");

    // TST
    copyFiles(pathInstall, pathOutTest, ".tst");
    copyFiles(pathData1, pathOutTest, ".tst");
    copyFiles(pathData2, pathOutTest, ".tst");

    return 0;
}

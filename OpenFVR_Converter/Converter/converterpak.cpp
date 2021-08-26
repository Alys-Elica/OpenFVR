#include "converterpak.h"

#include <bitset>
#include <fstream>
#include <iostream>

namespace ConverterPack {
    bool unpack(const std::string& fileName, const std::string& outputFolder)
    {
        std::ifstream istr;
        istr.open(fileName, std::ios::binary);

        if (!istr.is_open()) {
            return false;
        }

        char header[5] = { 0, 0, 0, 0, 0 };
        uint32_t fileSize;

        istr.read(header, 4);
        istr.read(reinterpret_cast<char*>(&fileSize), sizeof(fileSize));

        while (!istr.eof()) {
            char compFileName[16];          // Compressed file name
            uint32_t compressionLevel;
            uint32_t compressedSize;        // Compressed file size
            uint32_t uncompressedSize;      // Compressed file originalSize (uncompressed)

            istr.read(compFileName, 16);
            istr.read(reinterpret_cast<char*>(&compressionLevel), sizeof(compressionLevel));
            istr.read(reinterpret_cast<char*>(&compressedSize), sizeof(compressedSize));
            istr.read(reinterpret_cast<char*>(&uncompressedSize), sizeof(uncompressedSize));

            std::vector<uint8_t> compressedData(compressedSize);
            istr.read((char*)compressedData.data(), compressedSize);

            std::vector<uint8_t> uncompressedData(uncompressedSize);

            switch (compressionLevel) {
            case 3: {
                std::ofstream ostr;
                ostr.open(outputFolder + "/" + std::string(compFileName), std::ios::binary);
                if (!ostr.is_open()) {
                    std::cerr << "Can't open output file " << std::string(compFileName) << "\n";
                    continue;
                }

                int uncompressedCounter = 0;
                int compressedCounter = 0;
                while (uncompressedCounter < uncompressedSize) {
                    uint8_t byte = compressedData[compressedCounter++];

                    if (std::bitset<8>(byte)[7]) {
                        /*
                         * If most significant bit is set to 1
                         *      => copy of N1 bytes from already uncompressed data starting at N2 bytes from the end of
                         *         current outputed data
                         *              N1 = byte & 0x3f;
                         *              N2 = index (1 byte if second most significant bit is 1, 2 bytes if 0)
                        */

                        int size = (byte & 0x3f) + 1;
                        int index;
                        if (std::bitset<8>(byte)[6]) {
                            uint8_t tmpIndex = compressedData[compressedCounter++];
                            index = uncompressedCounter - (tmpIndex + 1);
                        }
                        else {
                            uint16_t tmpIndex = (compressedData[compressedCounter++] << 8) + compressedData[compressedCounter++];
                            index = uncompressedCounter - (tmpIndex + 1);
                        }
                        for (int i = 0; i < size; ++i) {
                            uncompressedData[uncompressedCounter++] = uncompressedData[index++];
                        }
                    }
                    else {
                        for (int count = 0; count < byte + 1; ++count) {
                            uncompressedData[uncompressedCounter++] = compressedData[compressedCounter++];
                        }
                    }
                }

                ostr.write((char*)uncompressedData.data(), uncompressedSize);
                ostr.close();

                break;
            }
            default:
                std::cerr << "Compression not yet known\n";
                continue;
            }

            istr.close();

            return true;
        }
    }
}

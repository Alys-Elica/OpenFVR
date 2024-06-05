#include "fvr_4xm.h"

#include <algorithm>
#include <cstring>
#include <iostream>
#include <vector>

#include "fvr/datastream.h"
#include "internal/dct.h"

/* Private */
class Fvr4xm::Fvr4xmPrivate {
    friend class Fvr4xm;

public:
    void logFileError(const std::string& message);

    uint32_t readChunkList();
    std::string readChunkString(const char* checkStr);

    bool parseVideoTrack();
    bool parseSoundTrack();

    bool readRiff();
    bool readHead();
    bool readTrk_();
    bool readMovi();

private:
    std::fstream m_file;

    std::string m_name;
    std::string m_info;
    uint32_t m_dataRate;
    uint32_t m_frameRate;
    int m_frameCount;

    std::vector<TrackVideo> m_videoTracks;
    std::vector<TrackSound> m_soundTracks;
};

void Fvr4xm::Fvr4xmPrivate::logFileError(const std::string& message)
{
    std::cerr << "[4XM](off: " << m_file.tellg() << ") - " << message << std::endl;
}

/**
 * Reads a chunk list from the file.
 * @return The size of the chunk list (0 if invalid).
 */
uint32_t Fvr4xm::Fvr4xmPrivate::readChunkList()
{
    char chunkList[5] = { 0 };
    m_file.read(chunkList, 4);
    if (strcmp(chunkList, "LIST") != 0) {
        logFileError("Invalid LIST header");
        return 0;
    }

    uint32_t chunkListSize = 0;
    m_file >> chunkListSize;

    return chunkListSize;
}

/**
 * Reads a chunk name from the file.
 * @return The chunk name.
 */
std::string Fvr4xm::Fvr4xmPrivate::readChunkString(const char* checkStr)
{
    char chunkName[5] = { 0 };
    m_file.read(chunkName, 4);

    if (strcmp(chunkName, checkStr) != 0) {
        return std::string();
    }

    uint32_t chunkNameSize = 0;
    m_file >> chunkNameSize;

    if (chunkNameSize % 2 != 0) {
        // There seem to be a pair number of bytes in the chunk name
        // We need to read an extra byte for padding
        ++chunkNameSize;
    }

    std::string name(chunkNameSize, '\0');
    m_file.read(name.data(), chunkNameSize);

    return name;
}

bool Fvr4xm::Fvr4xmPrivate::parseVideoTrack()
{
    // Video track name
    std::string name = readChunkString("name");

    // Video track info
    char vtrk[5] = { 0 };
    m_file.read(vtrk, 4);
    if (strcmp(vtrk, "vtrk") != 0) {
        logFileError("Invalid VTRK header");
        return false;
    }

    uint32_t vtrkSize = 0;
    m_file >> vtrkSize;

    char unknown[28] = { 0 };
    m_file.read(unknown, 28);

    uint32_t width = 0;
    m_file >> width;

    uint32_t height = 0;
    m_file >> height;

    uint32_t width2 = 0;
    m_file >> width2;

    uint32_t height2 = 0;
    m_file >> height2;

    char unknown2[24] = { 0 };
    m_file.read(unknown, 24);

    TrackVideo trackVideo;
    trackVideo.name = name;
    trackVideo.width = width;
    trackVideo.height = height;
    m_videoTracks.push_back(trackVideo);

    return true;
}

bool Fvr4xm::Fvr4xmPrivate::parseSoundTrack()
{
    // Sound track name
    std::string name = readChunkString("name");

    // Sound track info
    char strk[5] = { 0 };
    m_file.read(strk, 4);
    if (strcmp(strk, "strk") != 0) {
        logFileError("Invalid STRK header");
        return false;
    }

    uint32_t strkSize = 0;
    m_file >> strkSize;

    uint32_t trackNumber = 0;
    m_file >> trackNumber;

    uint32_t type = 0;
    m_file >> type;

    char unknown[20] = { 0 };
    m_file.read(unknown, 20);

    uint32_t channels = 0;
    m_file >> channels;

    uint32_t sampleRate = 0;
    m_file >> sampleRate;

    uint32_t sampleResolution = 0;
    m_file >> sampleResolution;

    TrackSound trackSound;
    trackSound.name = name;
    trackSound.trackNumber = trackNumber;
    trackSound.type = static_cast<Fvr4xm::AudioType>(type);
    trackSound.channels = channels;
    trackSound.sampleRate = sampleRate;
    trackSound.sampleResolution = sampleResolution;
    m_soundTracks.push_back(trackSound);

    return true;
}

bool Fvr4xm::Fvr4xmPrivate::readRiff()
{
    // RIFF header
    char riff[5] = { 0 };
    m_file.read(riff, 4);
    if (strcmp(riff, "RIFF") != 0) {
        logFileError("Invalid RIFF header");
        return false;
    }

    // File size
    uint32_t fileSize = 0;
    m_file >> fileSize;

    // Type
    char type[5] = { 0 };
    m_file.read(type, 4);
    if (strcmp(type, "4XMV") != 0) {
        logFileError("Invalid 4XM header");
        return false;
    }

    return true;
}

bool Fvr4xm::Fvr4xmPrivate::readHead()
{
    uint32_t chunkHeadListSize = readChunkList();
    if (chunkHeadListSize == 0) {
        logFileError("Invalid HEAD chunk list");
        return false;
    }

    // HEAD header
    char head[5] = { 0 };
    m_file.read(head, 4);
    if (strcmp(head, "HEAD") != 0) {
        logFileError("Invalid HEAD header");
        return false;
    }

    // HNFO
    uint32_t chunkHnfoListSize = readChunkList();
    if (chunkHnfoListSize == 0) {
        logFileError("Invalid HNFO chunk list");
        return false;
    }

    char hnfo[5] = { 0 };
    m_file.read(hnfo, 4);
    if (strcmp(hnfo, "HNFO") != 0) {
        logFileError("Invalid HNFO header");
        return false;
    }

    // HNFO name
    std::string name = readChunkString("name");

    // HNFO info
    std::string info = readChunkString("info");

    // HNFO std_
    char hnfoStd_[5] = { 0 };
    m_file.read(hnfoStd_, 4);
    if (strcmp(hnfoStd_, "std_") != 0) {
        logFileError("Invalid HNFO std_ header");
        return false;
    }

    uint32_t hnfoStd_Size = 0;
    m_file >> hnfoStd_Size;

    uint32_t dataRate = 0;
    m_file >> dataRate;

    uint32_t frameRate = 0;
    m_file >> frameRate;

    switch (frameRate) {
    case 0x41700000:
        frameRate = 15;
        break;
    case 0x41F00000:
        frameRate = 30;
        break;
    default:
        logFileError("Invalid/unsupported frame rate");
        return false;
    }

    m_name = name;
    m_info = info;
    m_dataRate = dataRate;
    m_frameRate = frameRate;

    return true;
}

bool Fvr4xm::Fvr4xmPrivate::readTrk_()
{
    uint32_t chunkTrk_ListSize = readChunkList();
    if (chunkTrk_ListSize == 0) {
        logFileError("Invalid TRK_ chunk list");
        return false;
    }

    size_t offsetTrk = m_file.tellg();

    // TRK_ header
    char trk_[5] = { 0 };
    m_file.read(trk_, 4);
    if (strcmp(trk_, "TRK_") != 0) {
        logFileError("Invalid TRK_ header");
        return false;
    }

    while (m_file.tellg() < offsetTrk + chunkTrk_ListSize) {
        uint32_t chunkSubTrkListSize = readChunkList();
        if (chunkSubTrkListSize == 0) {
            logFileError("Invalid ?TRK chunk list");
            return false;
        }

        // ?TRK header
        char subTrk[5] = { 0 };
        m_file.read(subTrk, 4);

        if (strcmp(subTrk, "VTRK") == 0) {
            if (!parseVideoTrack()) {
                return false;
            }
        } else if (strcmp(subTrk, "STRK") == 0) {
            if (!parseSoundTrack()) {
                return false;
            }
        } else {
            logFileError("Invalid ?TRK header");
            return false;
        }
    }

    m_file.seekg(offsetTrk + chunkTrk_ListSize);

    return true;
}

bool Fvr4xm::Fvr4xmPrivate::readMovi()
{
    uint32_t chunkMovi_ListSize = readChunkList();
    if (chunkMovi_ListSize == 0) {
        logFileError("Invalid MOVI chunk list");
        return false;
    }

    size_t offsetMovi = m_file.tellg();

    // MOVI header
    char movi[5] = { 0 };
    m_file.read(movi, 4);
    if (strcmp(movi, "MOVI") != 0) {
        logFileError("Invalid MOVI header");
        return false;
    }

    size_t offsetMoviData = m_file.tellg();
    m_frameCount = 0;
    while (m_file.tellg() < offsetMovi + chunkMovi_ListSize) {
        uint32_t chunkSubMoviListSize = readChunkList();
        if (chunkSubMoviListSize == 0) {
            logFileError("Invalid ?MOVI chunk list");
            return false;
        }

        size_t curFrameOffset = m_file.tellg();

        char fram[5] = { 0 };
        m_file.read(fram, 4);

        if (strcmp(fram, "FRAM") != 0) {
            logFileError("Invalid FRAM header");
            return false;
        }

        while (m_file.tellg() < curFrameOffset + chunkSubMoviListSize) {
            char chunkType[5] = { 0 };
            m_file.read(chunkType, 4);
            std::string chunkTypeStr(chunkType);

            uint32_t chunkSize = 0;
            m_file >> chunkSize;

            if (chunkTypeStr != "ifrm" && chunkTypeStr != "pfrm" && chunkTypeStr != "cfrm" && chunkTypeStr != "snd_") {
                logFileError("Invalid sub FRM chunk header");
                return false;
            }

            m_file.seekg(m_file.tellg() + (std::streampos)chunkSize);
        }

        ++m_frameCount;
    }

    m_file.seekg(offsetMoviData);

    return true;
}

/* Public */
Fvr4xm::Fvr4xm()
{
    d_ptr = new Fvr4xmPrivate;
}

Fvr4xm::~Fvr4xm()
{
    delete d_ptr;
}

bool Fvr4xm::open(const std::string& videoName)
{
    d_ptr->m_file.open(videoName, std::ios::in | std::ios::binary);
    if (!d_ptr->m_file.is_open()) {
        return false;
    }

    std::cout << "Reading header..." << std::endl;
    if (!d_ptr->readRiff()) {
        d_ptr->m_file.close();
        return false;
    }

    std::cout << "Reading HNFO..." << std::endl;
    if (!d_ptr->readHead()) {
        d_ptr->m_file.close();
        return false;
    }

    std::cout << "Reading TRK_..." << std::endl;
    if (!d_ptr->readTrk_()) {
        d_ptr->m_file.close();
        return false;
    }

    std::cout << "Reading MOVI..." << std::endl;
    if (!d_ptr->readMovi()) {
        d_ptr->m_file.close();
        return false;
    }

    return true;
}

void Fvr4xm::close()
{
    d_ptr->m_file.close();
}

bool Fvr4xm::isOpen() const
{
    return d_ptr->m_file.is_open();
}

void Fvr4xm::printInfo() const
{
    std::cout << "Video info:" << std::endl;
    std::cout << "    Name: " << d_ptr->m_name << std::endl;
    std::cout << "    Info: " << d_ptr->m_info << std::endl;
    std::cout << "    Data rate: " << d_ptr->m_dataRate << std::endl;
    std::cout << "    Frame rate: " << d_ptr->m_frameRate << " fps" << std::endl;
    std::cout << "    Frame count: " << d_ptr->m_frameCount << std::endl;
    std::cout << "    Video tracks:" << std::endl;
    for (const auto& track : d_ptr->m_videoTracks) {
        std::cout << "        Name: " << track.name << std::endl;
        std::cout << "        Width: " << track.width << std::endl;
        std::cout << "        Height: " << track.height << std::endl;
    }
    std::cout << "    Sound tracks:" << std::endl;
    for (const auto& track : d_ptr->m_soundTracks) {
        std::cout << "        Name: " << track.name << std::endl;
        std::cout << "        Track number: " << track.trackNumber << std::endl;
        std::cout << "        Type: " << track.type << std::endl;
        std::cout << "        Channels: " << track.channels << std::endl;
        std::cout << "        Sample rate: " << track.sampleRate << std::endl;
        std::cout << "        Sample resolution: " << track.sampleResolution << std::endl;
    }
}

int Fvr4xm::getWidth() const
{
    if (d_ptr->m_videoTracks.empty()) {
        return 0;
    }

    return d_ptr->m_videoTracks[0].width;
}

int Fvr4xm::getHeight() const
{
    if (d_ptr->m_videoTracks.empty()) {
        return 0;
    }

    return d_ptr->m_videoTracks[0].height;
}

int Fvr4xm::getFrameRate() const
{
    return d_ptr->m_frameRate;
}

int Fvr4xm::getFrameCount() const
{
    return d_ptr->m_frameCount;
}

bool Fvr4xm::hasSound() const
{
    return !d_ptr->m_soundTracks.empty();
}

const Fvr4xm::TrackSound& Fvr4xm::getTrackSound() const
{
    return d_ptr->m_soundTracks[0];
}

void imaAdpcmUncompress(
    uint8_t* input,
    size_t size,
    uint8_t* output,
    int initialPredictor,
    int initialIndex,
    int skip = 0)
{
    int ima_index_table[16] = {
        -1, -1, -1, -1, 2, 4, 6, 8,
        -1, -1, -1, -1, 2, 4, 6, 8
    };

    int ima_step_table[89] = {
        7, 8, 9, 10, 11, 12, 13, 14, 16, 17,
        19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
        50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
        130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
        337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
        876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
        2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
        5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
        15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
    };

    // Decode the IMA data (bottom nibble first)
    for (int i = 0; i < size; ++i) {
        int8_t byte = *input++;

        int8_t nibble1 = byte & 0x0F;
        int8_t nibble2 = (byte & 0xF0) >> 4;

        for (int i = 0; i < 2; ++i) {
            int step = ima_step_table[initialIndex];
            int stepIndex = initialIndex + ima_index_table[nibble1];
            stepIndex = std::clamp(stepIndex, 0, 88);

            int diff = ((2 * (nibble1 & 7) + 1) * step) >> 4;
            int predictor = initialPredictor;
            if (nibble1 & 8) {
                predictor -= diff;
            } else {
                predictor += diff;
            }

            predictor = std::clamp(predictor, -32768, 32767);

            initialPredictor = predictor;
            initialIndex = stepIndex;

            nibble1 = nibble2;

            // Write the predictor to the output
            *(output++) = (predictor & 0xFF);
            *(output++) = ((predictor >> 8) & 0xFF);
            output += skip;
        }
    }
}

bool Fvr4xm::readFrame(std::vector<uint16_t>& dataVideo, std::vector<uint8_t>& dataAudio)
{
    uint32_t chunkSubMoviListSize = d_ptr->readChunkList();
    if (chunkSubMoviListSize == 0) {
        d_ptr->logFileError("Invalid ?MOVI chunk list");
        return false;
    }

    size_t curFrameOffset = d_ptr->m_file.tellg();

    char fram[5] = { 0 };
    d_ptr->m_file.read(fram, 4);

    if (strcmp(fram, "FRAM") != 0) {
        d_ptr->logFileError("Invalid FRAM header");
        return false;
    }

    while (d_ptr->m_file.tellg() < curFrameOffset + chunkSubMoviListSize) {
        char chunkType[5] = { 0 };
        d_ptr->m_file.read(chunkType, 4);

        uint32_t chunkSize = 0;
        d_ptr->m_file >> chunkSize;

        std::vector<uint8_t> chunkData(chunkSize);
        d_ptr->m_file.read((char*)chunkData.data(), chunkSize);

        if (strcmp(chunkType, "ifrm") == 0) {
            DataStream ds(&chunkData);
            ds.setEndian(std::endian::little);

            uint32_t unkn1;
            ds >> unkn1;

            uint32_t bitstreamSize;
            ds >> bitstreamSize;

            std::vector<uint8_t> bitstream(bitstreamSize);
            ds.read(bitstreamSize, bitstream.data());

            uint32_t prefixStreamSize; // Size of the prefix stream divided by 4
            ds >> prefixStreamSize;

            uint32_t tokenCount;
            ds >> tokenCount;

            // Prefix stream
            std::vector<uint8_t> prefixStream(prefixStreamSize * 4);
            ds.read(prefixStreamSize * 4, prefixStream.data());

            // TODO
            if (d_ptr->m_videoTracks.empty()) {
                d_ptr->logFileError("No video track found");
                break;
            }
            dataVideo.assign(d_ptr->m_videoTracks[0].width * d_ptr->m_videoTracks[0].height, 0xFFFF);
        } else if (strcmp(chunkType, "pfrm") == 0) {
            // TODO
        } else if (strcmp(chunkType, "cfrm") == 0) {
            // TODO
        } else if (strcmp(chunkType, "snd_") == 0) {
            if (chunkData.size() < 8) {
                d_ptr->logFileError("Empty sound data");
                break;
            }

            DataStream ds(&chunkData);
            ds.setEndian(std::endian::little);

            uint32_t unkn; // Probably the track number (need to check)
            ds >> unkn;

            uint32_t soundSize;
            ds >> soundSize;

            if (d_ptr->m_soundTracks.empty()) {
                d_ptr->logFileError("No sound track found");
                break;
            }

            dataAudio.resize(soundSize);
            switch (d_ptr->m_soundTracks[0].type) {
            case AudioType::AT_PCM:
                ds.read(soundSize, dataAudio.data());
                break;
            case AudioType::AT_4X_IMA_ADPCM: {
                if (d_ptr->m_soundTracks[0].channels == 1) {
                    // Mono
                    int16_t initialPredictor;
                    ds >> initialPredictor;

                    int16_t initialIndex;
                    ds >> initialIndex;

                    imaAdpcmUncompress(
                        chunkData.data() + 12,
                        chunkData.size() - 12,
                        dataAudio.data(),
                        initialPredictor,
                        initialIndex);
                } else if (d_ptr->m_soundTracks[0].channels == 2) {
                    // Stereo
                    int16_t initialPredictorLeft;
                    ds >> initialPredictorLeft;

                    int16_t initialPredictorRight;
                    ds >> initialPredictorRight;

                    uint16_t initialIndexLeft;
                    ds >> initialIndexLeft;

                    uint16_t initialIndexRight;
                    ds >> initialIndexRight;

                    uint8_t* inputTmp = chunkData.data() + 16;
                    int size = (chunkData.size() - 16) / 2;
                    imaAdpcmUncompress(
                        inputTmp,
                        size,
                        dataAudio.data(),
                        initialPredictorLeft,
                        initialIndexLeft,
                        2);
                    imaAdpcmUncompress(
                        inputTmp + size,
                        size,
                        dataAudio.data() + 2,
                        initialPredictorRight,
                        initialIndexRight,
                        2);
                }

                break;
            }
            default:
                d_ptr->logFileError("Invalid/unsupported audio type");
                break;
            }
        } else {
            d_ptr->logFileError("Invalid ?FRM header");
            return false;
        }
    }

    return true;
}

#ifndef FVR_4XM_H
#define FVR_4XM_H

#include "libfvr_globals.h"

#include <cstdint>
#include <string>
#include <vector>

class LIBFVR_EXPORT Fvr4xm final {
public:
    enum AudioType {
        AT_PCM = 0,
        AT_4X_IMA_ADPCM = 1,
    };

    struct TrackVideo {
        std::string name;
        uint32_t width;
        uint32_t height;
    };

    struct TrackSound {
        std::string name;
        int trackNumber;
        AudioType type;
        int channels;
        int sampleRate;
        int sampleResolution;
    };

public:
    Fvr4xm();
    ~Fvr4xm();

    bool open(const std::string& videoName);
    void close();

    bool isOpen() const;

    void printInfo() const;

    int getWidth() const;
    int getHeight() const;

    int getFrameRate() const;
    int getFrameCount() const;

    bool hasSound() const;
    const TrackSound& getTrackSound() const;

    bool readFrame(std::vector<uint16_t>& dataVideo, std::vector<uint8_t>& dataAudio);

private:
    class Fvr4xmPrivate;
    Fvr4xmPrivate* d_ptr;
};

#endif // FVR_4XM_H

#ifndef ENGINE_AUDIO_H
#define ENGINE_AUDIO_H

#include <cstdint>
#include <string>

class Audio {
public:
    Audio();
    ~Audio();

    bool init();
    void deinit();

    void playSound(const std::string& soundFile, uint8_t volume, bool loop = false);
    void stopSound(const std::string& ambienceFile);
    bool isSoundPlaying(const std::string& ambienceFile) const;

    void pause();
    void resume();

private:
    class AudioPrivate;
    AudioPrivate* d_ptr;
};

#endif // ENGINE_AUDIO_H

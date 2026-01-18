

#include "audio.h"

#include <iostream>
#include <map>

#define MINIAUDIO_IMPLEMENTATION
#include <base/miniaudio.h>

#include <ofnx/tools/log.h>

#define AUDIO_DIR "data/audio/"

/* Private */
class Audio::AudioPrivate {
    friend class Audio;

private:
    bool m_isInit = false;
    ma_engine m_engine;

    ma_sound_group m_soundGroup;
    std::map<std::string, ma_sound*> m_soundList;
};

/* Public */
Audio::Audio()
{
    d_ptr = new AudioPrivate();
}

Audio::~Audio()
{
    delete d_ptr;
}

bool Audio::init()
{
    ma_result result = ma_engine_init(nullptr, &d_ptr->m_engine);
    if (result != MA_SUCCESS) {
        LOG_ERROR("Failed to initialize audio engine");
        return false;
    }

    ma_sound_group_init(
        &d_ptr->m_engine,
        0, // flags
        nullptr, // Parent group
        &d_ptr->m_soundGroup);

    d_ptr->m_isInit = true;

    return true;
}

void Audio::deinit()
{
    if (!d_ptr->m_isInit) {
        return;
    }

    for (auto& sound : d_ptr->m_soundList) {
        ma_sound_stop(sound.second);
        ma_sound_uninit(sound.second);
        delete sound.second;
    }
    d_ptr->m_soundList.clear();

    ma_sound_group_uninit(&d_ptr->m_soundGroup);
    ma_engine_uninit(&d_ptr->m_engine);

    d_ptr->m_isInit = false;
}

void Audio::playSound(const std::string& soundFile, uint8_t volume, bool loop)
{
    const std::string file = AUDIO_DIR + soundFile;

    ma_sound* sound = new ma_sound();
    ma_result result = ma_sound_init_from_file(
        &d_ptr->m_engine,
        file.c_str(),
        0, // flags
        &d_ptr->m_soundGroup,
        nullptr,
        sound);
    if (result != MA_SUCCESS) {
        LOG_ERROR("Failed to load sound: {}", soundFile);
        delete sound;
        return;
    }

    // Set volume [0, 255] -> [0.0, 1.0]
    ma_sound_set_volume(sound, volume / 255.0f);

    if (loop) {
        ma_sound_set_looping(sound, MA_TRUE);
    }

    result = ma_sound_start(sound);
    if (result != MA_SUCCESS) {
        LOG_ERROR("Failed to play sound: {}", soundFile);
        ma_sound_uninit(sound);
        delete sound;
    }

    d_ptr->m_soundList[soundFile] = sound;
}

void Audio::stopSound(const std::string& soundFile)
{
    if (!isSoundPlaying(soundFile)) {
        return;
    }

    ma_sound* sound = d_ptr->m_soundList[soundFile];
    ma_sound_stop(sound);
    ma_sound_uninit(sound);
    delete sound;
    d_ptr->m_soundList.erase(soundFile);
}

bool Audio::isSoundPlaying(const std::string& soundFile) const
{
    return d_ptr->m_soundList.find(soundFile) != d_ptr->m_soundList.end();
}

void Audio::pause()
{
    ma_engine_stop(&d_ptr->m_engine);
}

void Audio::resume()
{
    ma_engine_start(&d_ptr->m_engine);
}

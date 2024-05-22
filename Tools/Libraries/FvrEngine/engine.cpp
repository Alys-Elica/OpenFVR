#include "engine.h"

#include <algorithm>
#include <chrono>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <map>
#include <thread>

#include <SDL.h>
#include <fvr_files/fvr_4xm.h>
#include <fvr_files/fvr_script.h>
#include <fvr_files/fvr_tst.h>
#include <fvr_files/fvr_vr.h>
#include <fvrrenderer.h>

#include "engine/audio.h"
#include "engine/input.h"
#include "engine/render.h"
#include "engine/script.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

/* Constants */
#define ENGINE_DATA_PATH "data/"
#define ENGINE_FPS 30
#define WINDOW_FOV 1.0f
#define PI 3.14159265359f
#define MOUSE_SENSITIVITY 0.005f

/* Private */
class Engine::EnginePrivate {
    friend class Engine;

public:
    bool loadScript(const std::string& scriptFile);
    void onWarpEnter(const std::string& warpName);
    void onWarpZoneClick(const std::string& warpName, int zoneId);

    void executeBlock(const FvrScript::InstructionBlock& block);

    bool isPanoramic() const;
    void render();

private:
    Engine* parent;
    bool m_isInit = false;

#ifdef _WIN32
    std::chrono::steady_clock::time_point m_lastTime;
#else
    std::chrono::_V2::system_clock::time_point m_lastTime;
#endif

    // Engine objects
    Audio m_audio;
    Input m_input;
    Render m_render;

    // Data
    FvrScript m_script;
    std::map<std::string, ScriptFunction> m_functions;
    std::string m_dataPath;

    FvrVr m_fileVr;
    FvrTst m_fileTst;

    bool m_isRunning = true;

    std::string m_currentWarp;

    std::map<std::string, FvrScript::InstructionParam> m_stateValues;

    std::vector<uint16_t> m_frameBuffer;
    FvrRenderer m_vrRenderer;
    std::map<std::string, int> m_playingAnim;
    std::map<std::string, FvrVr::Animation> m_animData;

    std::map<int, std::string> m_keyWarp;
    std::map<int, std::string> m_warpZoneCursor; // TODO: better implementation

    // VR
    float m_yaw = 4.71f; // 270°
    float m_pitch = 1.57f; // 90°
    float m_roll = 0.0f; // 0°

    int m_pointedZone = -1;

    // Movie
    bool m_inMovieMode = false;
    std::vector<uint16_t> m_frameBufferMovie;
};

bool Engine::EnginePrivate::loadScript(const std::string& scriptFile)
{
    if (!m_script.parseLst(scriptFile)) {
        std::cerr << "Failed to parse script file: " << scriptFile << std::endl;
        return false;
    }
    return true;
}

void Engine::EnginePrivate::onWarpEnter(const std::string& warpName)
{
    FvrScript::InstructionBlock& block = m_script.getInitBlock(warpName);
    executeBlock(block);
}

void Engine::EnginePrivate::onWarpZoneClick(const std::string& warpName, int zoneId)
{
    FvrScript::InstructionBlock& block = m_script.getTestBlock(warpName, zoneId);
    executeBlock(block);
}

void Engine::EnginePrivate::executeBlock(const FvrScript::InstructionBlock& block)
{
    for (const FvrScript::Instruction& instruction : block) {
        if (instruction.name == "plugin") {
            executeBlock(instruction.subInstructions);
        } else if (instruction.name == "ifand" || instruction.name == "ifor") {
            // Check parameters
            bool exec = instruction.name == "ifand";
            for (const FvrScript::InstructionParam& param : instruction.params) {
                if (!std::holds_alternative<std::string>(param)) {
                    std::cerr << "Invalid parameter type for " << instruction.name << std::endl;
                    continue;
                }

                std::string flag = std::get<std::string>(param);
                double value = std::get<double>(parent->getStateValue(flag));

                if (instruction.name == "ifand") {
                    if (value == 0.0) {
                        exec = false;
                        break;
                    }
                } else {
                    if (value != 0.0) {
                        exec = true;
                        break;
                    }
                }
            }

            if (exec) {
                executeBlock(instruction.subInstructions);
            }
        } else if (instruction.name == "return") {
            return;
        } else if (instruction.name == "end") {
            parent->end();
        } else {
            if (m_functions.find(instruction.name) == m_functions.end()) {
                std::cerr << "Function not found: " << instruction.name << std::endl;
                continue;
            }

            m_functions[instruction.name](*parent, instruction.params);

            if (instruction.name == "gotowarp") {
                // TODO: check if this is the best way to handle this
                return;
            }
        }
    }
}

bool Engine::EnginePrivate::isPanoramic() const
{
    return m_fileVr.getType() == FvrVr::Type::VR_STATIC_VR;
}

void Engine::EnginePrivate::render()
{
    if (!m_inMovieMode) {
        // Update animations
        for (auto& animIt : m_playingAnim) {
            if (animIt.second < m_animData[animIt.first].frames.size() - 1) {
                animIt.second++;
            } else {
                animIt.second = 0;
            }

            // Apply frame
            const FvrVr::AnimationFrame& frame = m_animData.at(animIt.first).frames[animIt.second];
            for (int idxBlock = 0; idxBlock < frame.offsetList.size(); idxBlock++) {
                const uint32_t offset = frame.offsetList.at(idxBlock);
                const uint8_t* blockData = frame.data.data() + idxBlock * 64 * 2;

                for (int idxPixel = 0; idxPixel < 64; idxPixel++) {
                    const uint8_t lsb = blockData[idxPixel * 2];
                    const uint8_t msb = blockData[idxPixel * 2 + 1];

                    const uint16_t pixel = (msb << 8) | lsb;

                    if (isPanoramic()) {
                        unsigned short* buffer = m_vrRenderer.cubemapBuffer();
                        buffer[offset + m_fileVr.getWidth() * (idxPixel / 8) + (idxPixel % 8)] = pixel;
                    } else {
                        m_frameBuffer[offset + m_fileVr.getWidth() * (idxPixel / 8) + (idxPixel % 8)] = pixel;
                    }
                }
            }
        }

        // Render
        if (isPanoramic()) {
            // Draw to surface
            m_vrRenderer.render(
                m_frameBuffer.data(),
                m_yaw - 1.570795f, // Rotate 90 degrees
                m_pitch,
                m_roll,
                WINDOW_FOV);
        }
    }

    m_render.render(*parent);
}

/* Public */
Engine::Engine()
{
    d_ptr = new EnginePrivate();
    d_ptr->parent = this;

    av_log_set_level(AV_LOG_ERROR);
}

Engine::~Engine()
{
    delete d_ptr;
}

bool Engine::init()
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

    // Init engine objects
    if (!d_ptr->m_audio.init()) {
        std::cerr << "Failed to initialize audio" << std::endl;
        deinit();
        return false;
    }

    if (!d_ptr->m_render.init()) {
        std::cerr << "Failed to initialize render" << std::endl;
        deinit();
        return false;
    }

    if (!d_ptr->m_input.init()) {
        std::cerr << "Failed to initialize input" << std::endl;
        deinit();
        return false;
    }

    // Init data
    registerScriptFunctions(*this);
    d_ptr->m_dataPath = ENGINE_DATA_PATH;
    d_ptr->m_vrRenderer.setResolution(d_ptr->m_render.getWidth(), d_ptr->m_render.getHeight());
    d_ptr->m_isInit = true;

    return true;
}

void Engine::loop()
{
    // Load initial script
    if (!d_ptr->loadScript(d_ptr->m_dataPath + "script/script_1.lst")) {
        std::cerr << "Failed to load initial script" << std::endl;
        return;
    }

    const std::chrono::milliseconds frameDelay(1000 / ENGINE_FPS);

    int frameCount = 0;
    d_ptr->m_lastTime = std::chrono::high_resolution_clock::now();

    gotoWarp("init");

    while (d_ptr->m_isRunning) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - d_ptr->m_lastTime);

        if (elapsedTime >= frameDelay) {
            d_ptr->m_lastTime = currentTime;

            // Update
            std::vector<Input::Event> events = d_ptr->m_input.update();
            for (const Input::Event& event : events) {
                if (d_ptr->m_keyWarp.find(event.type) != d_ptr->m_keyWarp.end()) {
                    gotoWarp(d_ptr->m_keyWarp[event.type]);
                }

                switch (event.type) {
                case Input::Event::Type::Quit:
                    d_ptr->m_isRunning = false;
                    break;
                case Input::Event::Type::MouseMove:
                    if (isPanoramic()) {
                        d_ptr->m_yaw += event.xRel * MOUSE_SENSITIVITY;
                        d_ptr->m_pitch -= event.yRel * MOUSE_SENSITIVITY;

                        if (d_ptr->m_yaw < 0.0f) {
                            d_ptr->m_yaw = 2.0f * PI;
                        } else if (d_ptr->m_yaw > 2.0f * PI) {
                            d_ptr->m_yaw = 0.0f;
                        }

                        if (d_ptr->m_pitch < 0.0f) {
                            d_ptr->m_pitch = 0.0f;
                        } else if (d_ptr->m_pitch > 2.0f * PI) {
                            d_ptr->m_pitch = 0.0f;
                        }

                        d_ptr->m_pitch = std::clamp(d_ptr->m_pitch, 0.0f, PI);

                        d_ptr->m_pointedZone = d_ptr->m_fileTst.checkAngleZone(d_ptr->m_yaw, d_ptr->m_pitch);
                    } else {
                        d_ptr->m_pointedZone = d_ptr->m_fileTst.checkStaticZone((float)event.x, (float)event.y);
                    }
                    break;
                case Input::Event::Type::MouseClickLeft:
                    int zoneIndex;

                    if (isPanoramic()) {
                        zoneIndex = d_ptr->m_fileTst.checkAngleZone(d_ptr->m_yaw, d_ptr->m_pitch);
                    } else {
                        zoneIndex = d_ptr->m_fileTst.checkStaticZone((float)event.x, (float)event.y);
                    }

                    if (zoneIndex >= 0) {
                        d_ptr->onWarpZoneClick(d_ptr->m_currentWarp, zoneIndex);
                    }

                    break;
                }
            }

            d_ptr->render();
        } else {
            std::this_thread::sleep_for(frameDelay - elapsedTime);
        }
    }
}

void Engine::deinit()
{
    if (!d_ptr->m_isInit) {
        return;
    }

    if (d_ptr->m_fileVr.isOpen()) {
        d_ptr->m_fileVr.close();
    }

    d_ptr->m_audio.deinit();
    d_ptr->m_input.deinit();
    d_ptr->m_render.deinit();

    SDL_Quit();

    d_ptr->m_isInit = false;
}

void Engine::registerScriptFunction(const std::string& name, const ScriptFunction& function)
{
    if (d_ptr->m_functions.find(name) != d_ptr->m_functions.end()) {
        std::cerr << "Function already registered: " << name << std::endl;
        return;
    }

    d_ptr->m_functions[name] = function;
}

bool Engine::inMovieMode() const
{
    return d_ptr->m_inMovieMode;
}

bool Engine::isPanoramic() const
{
    return d_ptr->m_fileVr.getType() == FvrVr::Type::VR_STATIC_VR;
}

bool Engine::isOnZone() const
{
    return d_ptr->m_pointedZone >= 0;
}

int Engine::pointedZone() const
{
    return d_ptr->m_pointedZone;
}

std::vector<uint16_t>& Engine::getFrameBuffer()
{
    return inMovieMode() ? d_ptr->m_frameBufferMovie : d_ptr->m_frameBuffer;
}

void Engine::registerKeyWarp(int key, const std::string& warpName)
{
    // TODO: implement missing keys
    switch (key) {
    case 0: // Escape
        d_ptr->m_keyWarp.insert({ Input::Event::Type::MainMenu, warpName });
        break;
    case 12: // Right-click
        d_ptr->m_keyWarp.insert({ Input::Event::Type::MouseClickRight, warpName });
        break;
    default:
        std::cerr << "Invalid key for warp" << std::endl;
        break;
    }
}

void Engine::unregisterKeyWarp(int key)
{
    d_ptr->m_keyWarp.erase(key);
}

void Engine::clearKeyWarps()
{
    d_ptr->m_keyWarp.clear();
}

void Engine::setWarpZoneCursor(const std::string& warp, int zoneId, const std::string& cursorFile)
{
    d_ptr->m_warpZoneCursor.insert({ zoneId, cursorFile });
}

std::string Engine::getWarpZoneCursor(int zoneId) const
{
    if (d_ptr->m_warpZoneCursor.find(zoneId) != d_ptr->m_warpZoneCursor.end()) {
        return d_ptr->m_warpZoneCursor.at(zoneId);
    }

    return "";
}

void Engine::end()
{
    d_ptr->m_isRunning = false;
}

void Engine::gotoWarp(const std::string& warpName)
{
    // Clear previous data
    d_ptr->m_playingAnim.clear();
    d_ptr->m_currentWarp = warpName;
    d_ptr->m_warpZoneCursor.clear();

    if (d_ptr->m_fileVr.isOpen()) {
        d_ptr->m_fileVr.close();
    }

    const std::string warpVr = d_ptr->m_dataPath + "warp/" + warpName + ".vr";
    if (d_ptr->m_fileVr.open(warpVr)) {
        for (const std::string& anim : d_ptr->m_fileVr.getAnimationList()) {
            d_ptr->m_animData.insert({ anim, FvrVr::Animation() });
            d_ptr->m_fileVr.getAnimation(anim, d_ptr->m_animData[anim]);
        }

        if (d_ptr->isPanoramic()) {
            std::memcpy(d_ptr->m_vrRenderer.cubemapBuffer(), d_ptr->m_fileVr.getData().data(), d_ptr->m_fileVr.getData().size());
        } else {
            d_ptr->m_frameBuffer.resize(d_ptr->m_fileVr.getData().size() / 2);
            std::memcpy(d_ptr->m_frameBuffer.data(), d_ptr->m_fileVr.getData().data(), d_ptr->m_fileVr.getData().size());
        }

        // Load zones if available
        const std::string warpTst = d_ptr->m_dataPath + "tst/" + warpName + ".tst";
        if (!d_ptr->m_fileTst.loadFile(warpTst)) {
            // std::cerr << "Failed to load TST file" << std::endl;
            // return;
        }
    }

    d_ptr->onWarpEnter(d_ptr->m_currentWarp);
}

FvrScript::InstructionParam Engine::getStateValue(const std::string& key)
{
    std::string s = key;
    std::transform(s.begin(), s.end(), s.begin(),
        [](unsigned char c) { return std::tolower(c); });

    return d_ptr->m_stateValues[s];
}

void Engine::setStateValue(const std::string& key, const FvrScript::InstructionParam& value)
{
    std::string s = key;
    std::transform(s.begin(), s.end(), s.begin(),
        [](unsigned char c) { return std::tolower(c); });

    d_ptr->m_stateValues[s] = value;
}

void Engine::playAnim(const std::string& animName)
{
    d_ptr->m_playingAnim.insert({ animName, 0 }); // Stores the current frame
}

void Engine::playSound(const std::string& soundFile, uint8_t volume, bool loop)
{
    d_ptr->m_audio.playSound(soundFile, volume, loop);
}

void Engine::stopSound(const std::string& soundFile)
{
    d_ptr->m_audio.stopSound(soundFile);
}

void Engine::playMovie(const std::string& movieFile)
{
    // TODO: debug to skip movie playback, remove when done
    // return;

    std::string fileName = d_ptr->m_dataPath + "video/" + movieFile;

    // Init libavcodec
    AVFormatContext* formatContext = avformat_alloc_context();
    if (avformat_open_input(&formatContext, fileName.c_str(), NULL, NULL) != 0) {
        std::cout << "Unable to open file" << std::endl;
        return;
    }

    if (avformat_find_stream_info(formatContext, NULL) < 0) {
        avformat_close_input(&formatContext);
        std::cout << "Unable to find stream info" << std::endl;
        return;
    }

    // Video
    int videoStreamIndex = -1;
    for (int i = 0; i < formatContext->nb_streams; i++) {
        if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStreamIndex = i;
            break;
        }
    }

    if (videoStreamIndex == -1) {
        avformat_close_input(&formatContext);
        std::cout << "Unable to find video stream" << std::endl;
        return;
    }

    AVCodecParameters* localCodecParametersVideo = formatContext->streams[videoStreamIndex]->codecpar;
    const AVCodec* localCodecVideo = avcodec_find_decoder(localCodecParametersVideo->codec_id);
    if (localCodecVideo == NULL) {
        avformat_close_input(&formatContext);
        std::cout << "Unable to find codec" << std::endl;
        return;
    }

    AVCodecContext* codecContextVideo = avcodec_alloc_context3(localCodecVideo);
    if (avcodec_parameters_to_context(codecContextVideo, localCodecParametersVideo) < 0) {
        avformat_close_input(&formatContext);
        avcodec_close(codecContextVideo);
        std::cout << "Unable to copy codec parameters" << std::endl;
        return;
    }

    if (avcodec_open2(codecContextVideo, localCodecVideo, NULL) < 0) {
        avformat_close_input(&formatContext);
        avcodec_close(codecContextVideo);
        std::cout << "Unable to open codec" << std::endl;
        return;
    }

    // Audio
    int audioSreamIndex = -1;
    for (int i = 0; i < formatContext->nb_streams; i++) {
        if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioSreamIndex = i;
            break;
        }
    }

    SDL_AudioDeviceID deviceId = 0;
    AVCodecParameters* localCodecParametersAudio = NULL;
    AVCodec* localCodecAudio = NULL;
    AVCodecContext* codecContextAudio = NULL;
    if (audioSreamIndex >= 0) {
        std::cout << "Audio stream found" << std::endl;
        SDL_AudioSpec wav_spec;
        SDL_zero(wav_spec);
        wav_spec.freq = formatContext->streams[audioSreamIndex]->codecpar->sample_rate;
        wav_spec.format = AUDIO_S16SYS;
        wav_spec.channels = formatContext->streams[audioSreamIndex]->codecpar->channels;
        wav_spec.samples = 4096;

        // Ouvrir le périphérique audio
        deviceId = SDL_OpenAudioDevice(NULL, 0, &wav_spec, NULL, 0);
        if (deviceId == 0) {
            // Gérer l'erreur
            std::cerr << "SDL audio error: " << SDL_GetError() << std::endl;
            return;
        }

        SDL_PauseAudioDevice(deviceId, 0);

        localCodecParametersAudio = formatContext->streams[audioSreamIndex]->codecpar;
        // TODO: supposed to be const
        localCodecAudio = (AVCodec*)avcodec_find_decoder(localCodecParametersAudio->codec_id);
        if (localCodecAudio == NULL) {
            avformat_close_input(&formatContext);
            std::cout << "Unable to find codec" << std::endl;
            return;
        }

        codecContextAudio = avcodec_alloc_context3(localCodecAudio);
        if (avcodec_parameters_to_context(codecContextAudio, localCodecParametersAudio) < 0) {
            avformat_close_input(&formatContext);
            avcodec_close(codecContextVideo);
            avcodec_close(codecContextAudio);
            std::cout << "Unable to copy codec parameters" << std::endl;
            return;
        }

        if (avcodec_open2(codecContextAudio, localCodecAudio, NULL) < 0) {
            avformat_close_input(&formatContext);
            avcodec_close(codecContextVideo);
            avcodec_close(codecContextAudio);
            std::cout << "Unable to open codec" << std::endl;
            return;
        }
    }

    // Decoding
    AVPacket* packet = av_packet_alloc();
    AVFrame* frame = av_frame_alloc();

    d_ptr->m_inMovieMode = true;

    // Render video
    double frameRate = av_q2d(formatContext->streams[videoStreamIndex]->r_frame_rate);
    double waitTimeMs = 1000.0 / (double)frameRate;
    std::cout << "Frame rate: " << frameRate << std::endl;
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    while (av_read_frame(formatContext, packet) >= 0) {
        if (packet->stream_index == audioSreamIndex) {
            if (audioSreamIndex >= 0) {
                // Decode audio
                if (avcodec_send_packet(codecContextAudio, packet) < 0) {
                    continue;
                }

                if (avcodec_receive_frame(codecContextAudio, frame) < 0) {
                    continue;
                }

                // Play audio
                if (frame->nb_samples > 0) {
                    SDL_QueueAudio(deviceId, frame->data[0], frame->nb_samples * frame->channels * 2);
                }
            }
        }

        if (packet->stream_index == videoStreamIndex) {
            // Decode frame
            if (avcodec_send_packet(codecContextVideo, packet) < 0) {
                continue;
            }

            if (avcodec_receive_frame(codecContextVideo, frame) < 0) {
                continue;
            }

            if (frame->width != 640 || frame->height != 480) {
                std::cerr << "Invalid frame size" << std::endl;
                continue;
            }

            d_ptr->m_frameBufferMovie.resize(frame->width * frame->height);
            struct SwsContext* swsCtx = sws_getContext(
                frame->width, frame->height, (AVPixelFormat)frame->format, // source
                frame->width, frame->height, AV_PIX_FMT_RGB565, // destination
                SWS_BILINEAR, NULL, NULL, NULL);

            if (!swsCtx) {
                std::cerr << "Failed to create swscale context" << std::endl;
                return;
            }

            AVFrame* pFrameRGB565 = av_frame_alloc();
            pFrameRGB565->format = AV_PIX_FMT_RGB565;
            pFrameRGB565->width = frame->width;
            pFrameRGB565->height = frame->height;
            av_frame_get_buffer(pFrameRGB565, 0);

            sws_scale(swsCtx, frame->data, frame->linesize, 0, frame->height, pFrameRGB565->data, pFrameRGB565->linesize);
            std::memcpy(d_ptr->m_frameBufferMovie.data(), pFrameRGB565->data[0], frame->width * frame->height * 2);

            // Cleanup
            sws_freeContext(swsCtx);
            av_frame_free(&pFrameRGB565);

            d_ptr->render();

            // Wait for next frame
            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
            std::chrono::milliseconds elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            if (elapsed.count() < waitTimeMs) {
                std::this_thread::sleep_for(std::chrono::milliseconds((int)(waitTimeMs - elapsed.count())));
            }
            start = std::chrono::steady_clock::now();
        }

        av_packet_unref(packet);

        // Event
        bool exit = false;
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_KEYDOWN || event.type == SDL_MOUSEBUTTONDOWN) {
                exit = true;
                break;
            }
        }

        if (exit) {
            break;
        }
    }

    av_frame_free(&frame);
    av_packet_free(&packet);
    avcodec_close(codecContextVideo);
    if (audioSreamIndex != NULL) {
        SDL_CloseAudioDevice(deviceId);
    }
    avformat_close_input(&formatContext);

    d_ptr->m_inMovieMode = false;
}

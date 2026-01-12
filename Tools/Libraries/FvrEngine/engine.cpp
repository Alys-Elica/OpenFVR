#include "engine.h"

#include <algorithm>
#include <chrono>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <map>
#include <set>
#include <thread>

#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>
#include <SDL3_image/SDL_image.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

#include <ofnx/files/tst.h>
#include <ofnx/files/vr.h>
#include <ofnx/graphics/rendereropengl.h>

#include "engine/audio.h"
#include "engine/eventmanager.h"

/* Constants */
#define ENGINE_DATA_PATH "data/"
#define ENGINE_FPS 30
#define ENGINE_WIDTH 640
#define ENGINE_HEIGHT 480
#define WINDOW_FOV 1.0f
#define MOUSE_SENSITIVITY 0.1f

/* Script functions */
void fvrGotoWarp(Engine& engine, std::vector<std::string> args)
{
    if (args.size() != 1) {
        std::cerr << "fvrGotoWarp: invalid argument count" << std::endl;
        return;
    }

    std::string warpId = args[0];

    engine.gotoWarp(warpId);
}

void fvrPlaySound(Engine& engine, std::vector<std::string> args)
{
    if (args.size() != 3) {
        std::cerr << "fvrPlaySound: invalid argument count" << std::endl;
        return;
    }

    std::string sound = args[0];
    double volume = std::stod(args[1]);
    double loop = std::stod(args[2]);

    // To lowercase
    std::transform(sound.begin(), sound.end(), sound.begin(), ::tolower);

    engine.playSound(sound, (int)volume, loop == -1);
}

void fvrStopSound(Engine& engine, std::vector<std::string> args)
{
    if (args.size() != 1) {
        std::cerr << "fvrStopSound: invalid argument count" << std::endl;
        return;
    }

    std::string sound = args[0];

    // To lowercase
    std::transform(sound.begin(), sound.end(), sound.begin(), ::tolower);

    engine.stopSound(sound);
}

void fvrPlaySound3d(Engine& engine, std::vector<std::string> args)
{
    if (args.size() != 4) {
        std::cerr << "fvrPlaySound3d: invalid argument count" << std::endl;
        return;
    }

    std::string sound = args[0];
    double x = std::stod(args[1]);
    double y = std::stod(args[2]);
    double z = std::stod(args[3]);

    // TODO: implement properly
    engine.playSound(sound, 100, false);
}

void fvrStopSound3d(Engine& engine, std::vector<std::string> args)
{
    if (args.size() != 1) {
        std::cerr << "fvrStopSound3d: invalid argument count" << std::endl;
        return;
    }

    std::string sound = args[0];

    // TODO: implement properly
    engine.stopSound(sound);
}

void fvrPlayMusic(Engine& engine, std::vector<std::string> args)
{
    if (args.size() != 1) {
        std::cerr << "fvrPlayMusic: invalid argument count" << std::endl;
        return;
    }

    std::string music = args[0];

    // TODO: implement properly
    engine.playSound(music, 100, true);
}

void fvrStopMusic(Engine& engine, std::vector<std::string> args)
{
    if (args.size() != 1) {
        std::cerr << "fvrStopMusic: invalid argument count" << std::endl;
        return;
    }

    std::string music = args[0];

    engine.stopSound(music);
}

void fvrSet(Engine& engine, std::vector<std::string> args)
{
    if (args.size() != 2) {
        std::cerr << "fvrSet: invalid argument count" << std::endl;
        return;
    }

    std::string flag = args[0];
    double value = std::stod(args[1]);

    engine.setStateValue(flag, std::to_string(value));
}

void fvrLockKey(Engine& engine, std::vector<std::string> args)
{
    if (args.size() != 2) {
        std::cerr << "fvrLockKey: invalid argument count" << std::endl;
        return;
    }

    /*
     * 0: Esc
     * 12: Right-click
     */
    double key = std::stod(args[0]);

    // TODO: better way to handle this ?
    try {
        if (std::stod(args[1]) == 0) {
            engine.unregisterKeyWarp((int)key);
        }
    } catch (const std::exception&) {
        std::string warp = args[1];
        // Remove trailing '.vr' from warp name
        // warp = warp.substr(0, warp.size() - 3);

        // To lowercase
        std::transform(warp.begin(), warp.end(), warp.begin(), ::tolower);

        engine.registerKeyWarp((int)key, warp);
    }
}

void fvrResetLockKey(Engine& engine, std::vector<std::string> args)
{
    engine.clearKeyWarps();
}

void fvrSetCursor(Engine& engine, std::vector<std::string> args)
{
    if (args.size() != 3) {
        std::cerr << "fvrSetCursor: invalid argument count" << std::endl;
        return;
    }

    std::string cursor = args[0];
    std::string warp = args[1];
    double zone = std::stod(args[2]);

    // To lowercase
    std::transform(cursor.begin(), cursor.end(), cursor.begin(), ::tolower);
    std::transform(warp.begin(), warp.end(), warp.begin(), ::tolower);

    engine.setWarpZoneCursor(warp, (int)zone, cursor);
}

void fvrSetCursorDefault(Engine& engine, std::vector<std::string> args)
{
    if (args.size() != 2) {
        std::cerr << "fvrSetCursorDefault: invalid argument count" << std::endl;
        return;
    }

    double value = std::stod(args[0]);
    std::string cursor = args[1];

    engine.setDefaultCursor(value, cursor);
}

void fvrFade(Engine& engine, std::vector<std::string> args)
{
    if (args.size() != 3) {
        std::cerr << "fvrFade: invalid argument count" << std::endl;
        return;
    }

    double start = std::stod(args[0]);
    double end = std::stod(args[1]);
    double timer = std::stod(args[2]);

    engine.fade(start, end, timer);
}

void fvrEnd(Engine& engine, std::vector<std::string> args)
{
    engine.end();
}

void fvrSetAngle(Engine& engine, std::vector<std::string> args)
{
    if (args.size() != 2) {
        std::cerr << "fvrSetAngle: invalid argument count" << std::endl;
        return;
    }

    int pitchInt = (int)std::stod(args[0]) & 0x1fff;
    int yawInt = (int)std::stod(args[1]) & 0x1fff;

    if (0xfff < (uint)pitchInt) {
        pitchInt = pitchInt - 0x2000;
    }

    float pitch = pitchInt * 360.0f / 8192.0f; // Convert from 0-8192 range to degree
    float yaw = yawInt * 360.0f / 8192.0f; // Convert from 0-8192 range to degree

    pitch += 90;

    engine.setAngle(pitch, yaw);
}

void fvrHideCursor(Engine& engine, std::vector<std::string> args)
{
    if (args.size() != 2) {
        std::cerr << "fvrHideCursor: invalid argument count" << std::endl;
        return;
    }

    std::string value1 = args[0];
    double value2 = std::stod(args[1]);

    // TODO: implement
    std::cout << "fvrHideCursor: not implemented: " << value1 << " " << value2 << std::endl;
}

void fvrNot(Engine& engine, std::vector<std::string> args)
{
    if (args.size() != 1) {
        std::cerr << "fvrNot: invalid argument count" << std::endl;
        return;
    }

    std::string value = args[0];
    double val = std::stod(engine.getStateValue(value));
    engine.setStateValue(value, val == 0.0 ? "1.0" : "0.0");
}

void fvrAngleXMax(Engine& engine, std::vector<std::string> args)
{
    if (args.size() != 1) {
        std::cerr << "fvrAngleXMax: invalid argument count" << std::endl;
        return;
    }

    double value = std::stod(args[0]);

    // TODO: implement
    std::cout << "fvrAngleXMax: not implemented: " << value << std::endl;
}

void fvrAngleYMax(Engine& engine, std::vector<std::string> args)
{
    if (args.size() != 1) {
        std::cerr << "fvrAngleYMax: invalid argument count" << std::endl;
        return;
    }

    double value = std::stod(args[0]);

    // TODO: implement
    std::cout << "fvrAngleXMax: not implemented: " << value << std::endl;
}

void fvrSetZoom(Engine& engine, std::vector<std::string> args)
{
    if (args.size() != 1) {
        std::cerr << "fvrSetZoom: invalid argument count" << std::endl;
        return;
    }

    double value = std::stod(args[0]);

    // TODO: implement
    std::cout << "fvrSetZoom: not implemented: " << value << std::endl;
}

void fvrInterpolAngle(Engine& engine, std::vector<std::string> args)
{
    if (args.size() != 3) {
        std::cerr << "fvrInterpolAngle: invalid argument count" << std::endl;
        return;
    }

    double value1 = std::stod(args[0]);
    double value2 = std::stod(args[1]);
    double value3 = std::stod(args[2]);

    // TODO: implement
    std::cout << "fvrInterpolAngle: not implemented: " << value1 << " " << value2 << " " << value3 << std::endl;
}

/* Private */
class Engine::EnginePrivate {
    friend class Engine;

public:
    enum class CursorSystem {
        Default,
        Pointer,
    };

public:
    bool loadScript(const std::string& scriptFile);
    void registerScriptFunction(const std::string& name, const ScriptFunction& function);

    void onWarpEnter(const std::string& warpName);
    void onWarpZoneClick(const std::string& warpName, int zoneId);

    void executeBlock(const ofnx::files::Lst::InstructionBlock& block);
    void executeBlockPlugin(const ofnx::files::Lst::InstructionBlock& block);

    bool isPanoramic() const;
    void render();

    void setCursorSettings(bool visible, bool centerLocked);
    void setCursorSystem(const CursorSystem& cursor);
    void setCursor(const std::string& cursorFile);

private:
    Engine* parent;
    bool m_isInit = false;

#ifdef _WIN32
    std::chrono::steady_clock::time_point m_lastTime;
#else
    std::chrono::_V2::system_clock::time_point m_lastTime;
#endif

    // Engine objects
    ofnx::graphics::RendererOpenGL m_rendererOgl;
    Audio m_audio;
    EventManager m_event;

    SDL_Window* m_window = nullptr;
    SDL_GLContext m_glContext;

    CursorSystem m_cursorCurrent;
    SDL_Cursor* m_cursor = nullptr;

    // Data
    ofnx::files::Lst m_script;
    std::map<std::string, ScriptFunction> m_functions;
    std::map<std::string, ScriptFunction> m_functionsPlugin;
    std::string m_dataPath;

    ofnx::files::Vr m_fileVr;
    ofnx::files::Tst m_fileTst;

    bool m_isRunning = true;

    std::string m_currentWarp;

    std::map<std::string, std::string> m_stateValues;

    std::vector<uint16_t> m_vrImageData;
    std::set<std::string> m_playingAnim;

    std::map<int, std::string> m_keyWarp;
    std::map<int, std::string> m_defaultCursor; // TODO: better implementation
    std::map<int, std::string> m_warpZoneCursor; // TODO: better implementation

    // VR
    float m_yaw = 270.0f;
    float m_pitch = 90.0f;
    float m_roll = 0.0f;

    int m_pointedZone = -1;
};

bool Engine::EnginePrivate::loadScript(const std::string& scriptFile)
{
    if (!m_script.parseLst(scriptFile)) {
        std::cerr << "Failed to parse script file: " << scriptFile << std::endl;
        return false;
    }

    // Init state values
    m_stateValues.clear();
    for (const std::string& variable : m_script.getVariables()) {
        parent->setStateValue(variable, "0");
    }

    return true;
}

void Engine::EnginePrivate::registerScriptFunction(const std::string& name, const ScriptFunction& function)
{
    if (m_functions.find(name) != m_functions.end()) {
        std::cerr << "Function already registered: " << name << std::endl;
        return;
    }

    m_functions[name] = function;
}

void Engine::EnginePrivate::onWarpEnter(const std::string& warpName)
{
    const ofnx::files::Lst::InstructionBlock& block = m_script.getInitBlock(warpName);
    executeBlock(block);
}

void Engine::EnginePrivate::onWarpZoneClick(const std::string& warpName, int zoneId)
{
    const ofnx::files::Lst::InstructionBlock& block = m_script.getTestBlock(warpName, zoneId);
    executeBlock(block);
}

void Engine::EnginePrivate::executeBlock(const ofnx::files::Lst::InstructionBlock& block)
{
    try {
        for (const ofnx::files::Lst::Instruction& instruction : block) {
            if (instruction.name == "plugin") {
                executeBlockPlugin(instruction.subInstructions);
            } else if (instruction.name == "ifand" || instruction.name == "ifor") {
                // Check parameters
                bool exec = instruction.name == "ifand";
                for (const std::string& param : instruction.params) {
                    double value = std::stod(parent->getStateValue(param));

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
    } catch (const std::exception& e) {
        std::cerr << "Error during script execution" << std::endl;
        m_isRunning = false;

        for (const auto& [key, val] : m_stateValues) {
            std::cout << key << ": " << val << std::endl;
        }
    }
}

void Engine::EnginePrivate::executeBlockPlugin(const ofnx::files::Lst::InstructionBlock& block)
{
    try {
        for (const ofnx::files::Lst::Instruction& instruction : block) {
            if (m_functionsPlugin.find(instruction.name) == m_functionsPlugin.end()) {
                std::cerr << "Function not found: " << instruction.name << std::endl;
                continue;
            }

            m_functionsPlugin[instruction.name](*parent, instruction.params);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error during script execution" << std::endl;
        m_isRunning = false;

        for (const auto& [key, val] : m_stateValues) {
            std::cout << key << ": " << val << std::endl;
        }
    }
}

bool Engine::EnginePrivate::isPanoramic() const
{
    return m_fileVr.getType() == ofnx::files::Vr::Type::VR_STATIC_VR;
}

void Engine::EnginePrivate::render()
{
    // Update animations
    for (const std::string& animName : m_playingAnim) {
        m_fileVr.applyAnimationFrameRgb565(animName, m_vrImageData.data());
    }

    // Render
    if (isPanoramic()) {
        int width;
        int height;
        SDL_GetWindowSize(m_window, &width, &height);
        m_rendererOgl.updateVr(m_vrImageData.data());
        m_rendererOgl.renderVr(width, height, m_yaw, m_pitch, m_roll, WINDOW_FOV);
        SDL_GL_SwapWindow(m_window);
    } else {
        m_rendererOgl.updateFrame(m_vrImageData.data());
        m_rendererOgl.renderFrame();
        SDL_GL_SwapWindow(m_window);
    }
}

void Engine::EnginePrivate::setCursorSettings(bool visible, bool centerLocked)
{
    if (centerLocked) {
        int width;
        int height;
        SDL_GetWindowSize(m_window, &width, &height);
        SDL_WarpMouseInWindow(m_window, width / 2, height / 2);
    }

    SDL_SetHint(SDL_HINT_MOUSE_RELATIVE_CURSOR_VISIBLE, visible ? "1" : "0");
    SDL_SetHint(SDL_HINT_MOUSE_RELATIVE_MODE_CENTER, centerLocked ? "1" : "0");
    SDL_SetWindowRelativeMouseMode(m_window, centerLocked);
}

void Engine::EnginePrivate::setCursorSystem(const CursorSystem& cursor)
{
    if (cursor == m_cursorCurrent) {
        return;
    }

    if (m_cursor) {
        SDL_DestroyCursor(m_cursor);
    }

    m_cursorCurrent = cursor;

    switch (cursor) {
    case CursorSystem::Pointer:
        m_cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_POINTER);
        break;

    default:
        m_cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT);
        break;
    }

    SDL_SetCursor(m_cursor);
}

void Engine::EnginePrivate::setCursor(const std::string& cursorFile)
{
    SDL_Surface* cursorSurface = IMG_Load(cursorFile.c_str());
    if (!cursorSurface) {
        std::cerr << "Failed to load image: " << SDL_GetError() << std::endl;
        return;
    }

    const SDL_PixelFormatDetails* formatDetails = SDL_GetPixelFormatDetails(cursorSurface->format);
    if (!formatDetails) {
        std::cerr << "Failed to get format details " << SDL_GetError() << std::endl;
        SDL_DestroySurface(cursorSurface);
        return;
    }

    Uint32 key = SDL_MapRGBA(formatDetails, NULL, 0, 0, 0, 0);
    SDL_SetSurfaceColorKey(cursorSurface, true, key);

    int hotX = cursorSurface->w / 2;
    int hotY = cursorSurface->h / 2;
    SDL_Cursor* cursor = SDL_CreateColorCursor(cursorSurface, hotX, hotY);

    SDL_DestroySurface(cursorSurface);

    if (!cursor) {
        std::cerr << "Failed to create SDL cursor: " << SDL_GetError() << std::endl;
        return;
    }

    if (m_cursor) {
        SDL_DestroyCursor(m_cursor);
    }
    m_cursor = cursor;

    SDL_SetCursor(m_cursor);
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
    // Init SDL3
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        std::cerr << "SDL_Init failed - " << SDL_GetError() << std::endl;
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    d_ptr->m_window = SDL_CreateWindow("FnxVR", ENGINE_WIDTH, ENGINE_HEIGHT, SDL_WINDOW_OPENGL);
    if (!d_ptr->m_window) {
        std::cerr << "Failed to create SDL window: " << SDL_GetError() << std::endl;
        return false;
    }

    d_ptr->m_glContext = SDL_GL_CreateContext(d_ptr->m_window);
    if (!d_ptr->m_glContext) {
        std::cerr << "Failed to create GL context: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(d_ptr->m_window);
        return false;
    }

    SDL_SetWindowRelativeMouseMode(d_ptr->m_window, true);

    // Disable VSync
    SDL_GL_SetSwapInterval(0);

    // Init engine objects
    // TODO: manage old/new VR version
    if (!d_ptr->m_rendererOgl.init(ENGINE_WIDTH, ENGINE_HEIGHT, false, (ofnx::graphics::RendererOpenGL::oglLoadFunc)SDL_GL_GetProcAddress)) {
        std::cerr << "Failed to initialize renderer" << std::endl;
        SDL_GL_DestroyContext(d_ptr->m_glContext);
        SDL_DestroyWindow(d_ptr->m_window);
        return false;
    }

    if (!d_ptr->m_audio.init()) {
        std::cerr << "Failed to initialize audio" << std::endl;
        d_ptr->m_rendererOgl.deinit();
        SDL_GL_DestroyContext(d_ptr->m_glContext);
        SDL_DestroyWindow(d_ptr->m_window);
        return false;
    }

    if (!d_ptr->m_event.init()) {
        std::cerr << "Failed to initialize audio" << std::endl;
        d_ptr->m_audio.deinit();
        d_ptr->m_rendererOgl.deinit();
        SDL_GL_DestroyContext(d_ptr->m_glContext);
        SDL_DestroyWindow(d_ptr->m_window);
        return false;
    }

    // Init data
    d_ptr->m_dataPath = ENGINE_DATA_PATH;
    d_ptr->m_isInit = true;

    d_ptr->registerScriptFunction("gotowarp", &fvrGotoWarp);
    d_ptr->registerScriptFunction("playsound", &fvrPlaySound);
    d_ptr->registerScriptFunction("stopsound", &fvrStopSound);
    d_ptr->registerScriptFunction("playsound3d", &fvrPlaySound3d);
    d_ptr->registerScriptFunction("stopsound3d", &fvrStopSound3d);
    d_ptr->registerScriptFunction("playmusique", &fvrPlayMusic);
    d_ptr->registerScriptFunction("stopmusique", &fvrStopMusic);
    d_ptr->registerScriptFunction("set", &fvrSet);
    d_ptr->registerScriptFunction("lockkey", &fvrLockKey);
    d_ptr->registerScriptFunction("resetlockkey", &fvrResetLockKey);
    d_ptr->registerScriptFunction("setcursor", &fvrSetCursor);
    d_ptr->registerScriptFunction("setcursordefault", &fvrSetCursorDefault);
    d_ptr->registerScriptFunction("fade", &fvrFade);
    d_ptr->registerScriptFunction("end", &fvrEnd);
    d_ptr->registerScriptFunction("setangle", &fvrSetAngle);
    d_ptr->registerScriptFunction("hidecursor", &fvrHideCursor);
    d_ptr->registerScriptFunction("not", &fvrNot);
    d_ptr->registerScriptFunction("anglexmax", &fvrAngleXMax);
    d_ptr->registerScriptFunction("angleymax", &fvrAngleYMax);
    d_ptr->registerScriptFunction("setzoom", &fvrSetZoom);
    d_ptr->registerScriptFunction("interpolangle", &fvrInterpolAngle);

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

    gotoWarp("init.vr");

    while (d_ptr->m_isRunning) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - d_ptr->m_lastTime);

        int pointedZone = d_ptr->m_pointedZone;

        if (elapsedTime >= frameDelay) {
            d_ptr->m_lastTime = currentTime;

            // Update
            std::vector<EventManager::Event> events = d_ptr->m_event.getEvents();
            for (const EventManager::Event& event : events) {
                if (d_ptr->m_keyWarp.find(event.type) != d_ptr->m_keyWarp.end()) {
                    gotoWarp(d_ptr->m_keyWarp[event.type]);
                }

                switch (event.type) {
                case EventManager::Event::Type::Quit:
                    d_ptr->m_isRunning = false;
                    break;
                case EventManager::Event::Type::MouseMove:
                    if (isPanoramic()) {
                        d_ptr->m_yaw += event.xRel * MOUSE_SENSITIVITY;
                        d_ptr->m_pitch -= event.yRel * MOUSE_SENSITIVITY;

                        if (d_ptr->m_yaw < 0.0f) {
                            d_ptr->m_yaw = 360.0f;
                        } else if (d_ptr->m_yaw > 360.0f) {
                            d_ptr->m_yaw = 0.0f;
                        }

                        if (d_ptr->m_pitch < 0.0f) {
                            d_ptr->m_pitch = 0.0f;
                        } else if (d_ptr->m_pitch > 360.0f) {
                            d_ptr->m_pitch = 0.0f;
                        }

                        d_ptr->m_pitch = std::clamp(d_ptr->m_pitch, 0.0f, 180.0f);

                        pointedZone = d_ptr->m_fileTst.checkZoneVr(d_ptr->m_yaw, d_ptr->m_pitch);
                    } else {
                        pointedZone = d_ptr->m_fileTst.checkZoneStatic((float)event.x, (float)event.y);
                    }
                    break;
                case EventManager::Event::Type::MouseClickLeft:
                    int zoneIndex;

                    if (isPanoramic()) {
                        zoneIndex = d_ptr->m_fileTst.checkZoneVr(d_ptr->m_yaw, d_ptr->m_pitch);
                    } else {
                        zoneIndex = d_ptr->m_fileTst.checkZoneStatic((float)event.x, (float)event.y);
                    }

                    if (zoneIndex >= 0) {
                        d_ptr->onWarpZoneClick(d_ptr->m_currentWarp, zoneIndex);
                    }

                    break;
                }
            }

            if (pointedZone != d_ptr->m_pointedZone) {
                d_ptr->m_pointedZone = pointedZone;

                if (d_ptr->m_warpZoneCursor.contains(d_ptr->m_pointedZone)) {
                    d_ptr->setCursor(ENGINE_DATA_PATH "image/" + d_ptr->m_warpZoneCursor[d_ptr->m_pointedZone]);
                } else {
                    if (d_ptr->m_pointedZone == -1) {
                        if (d_ptr->m_defaultCursor.contains(0)) {
                            d_ptr->setCursor(ENGINE_DATA_PATH "image/" + d_ptr->m_defaultCursor[0]);
                        } else {
                            d_ptr->setCursorSystem(EnginePrivate::CursorSystem::Default);
                        }
                    } else {
                        if (d_ptr->m_defaultCursor.contains(1)) {
                            d_ptr->setCursor(ENGINE_DATA_PATH "image/" + d_ptr->m_defaultCursor[1]);
                        } else {
                            d_ptr->setCursorSystem(EnginePrivate::CursorSystem::Default);
                        }
                    }
                }

                SDL_SetWindowTitle(d_ptr->m_window, std::string("Pointed zone: " + std::to_string(d_ptr->m_pointedZone)).c_str());
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

    d_ptr->m_audio.deinit();
    d_ptr->m_event.deinit();
    d_ptr->m_rendererOgl.deinit();

    if (d_ptr->m_cursor) {
        SDL_DestroyCursor(d_ptr->m_cursor);
        d_ptr->m_cursor = nullptr;
    }

    SDL_GL_DestroyContext(d_ptr->m_glContext);
    SDL_DestroyWindow(d_ptr->m_window);
    SDL_Quit();

    d_ptr->m_isInit = false;
}

void Engine::registerScriptPluginFunction(const std::string& name, const ScriptFunction& function)
{
    if (d_ptr->m_functionsPlugin.find(name) != d_ptr->m_functionsPlugin.end()) {
        std::cerr << "Function already registered: " << name << std::endl;
        return;
    }

    d_ptr->m_functionsPlugin[name] = function;
}

bool Engine::isPanoramic() const
{
    return d_ptr->isPanoramic();
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
    return d_ptr->m_vrImageData;
}

void Engine::registerKeyWarp(int key, const std::string& warpName)
{
    // TODO: implement missing keys
    switch (key) {
    case 0: // Escape
        d_ptr->m_keyWarp.insert({ EventManager::Event::Type::MainMenu, warpName });
        break;
    case 12: // Right-click
        d_ptr->m_keyWarp.insert({ EventManager::Event::Type::MouseClickRight, warpName });
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
    d_ptr->m_fileVr.clear();

    const std::string warpVr = d_ptr->m_dataPath + "warp/" + d_ptr->m_currentWarp;
    if (d_ptr->m_fileVr.load(warpVr)) {
        if (!d_ptr->m_fileVr.getDataRgb565(d_ptr->m_vrImageData)) {
            std::cerr << "Failed to load VR image data" << std::endl;
            return;
        }

        if (isPanoramic()) {
            d_ptr->setCursorSettings(true, true);
        } else {
            d_ptr->setCursorSettings(true, false);
        }

        // Load zones if available
        // Remove '.vr' if it exists
        std::string tmpWarpName = d_ptr->m_currentWarp;
        if (tmpWarpName.find(".vr") != std::string::npos) {
            tmpWarpName = tmpWarpName.substr(0, tmpWarpName.find(".vr"));
        }

        const std::string warpTst = d_ptr->m_dataPath + "tst/" + tmpWarpName + ".tst";
        if (!d_ptr->m_fileTst.loadFile(warpTst)) {
            // std::cerr << "Failed to load TST file" << std::endl;
            // return;
        }
    }

    d_ptr->onWarpEnter(d_ptr->m_currentWarp);
}

std::string Engine::getStateValue(const std::string& key)
{
    std::string s = key;
    std::transform(s.begin(), s.end(), s.begin(),
        [](unsigned char c) { return std::tolower(c); });

    return d_ptr->m_stateValues[s];
}

void Engine::setStateValue(const std::string& key, const std::string& value)
{
    std::string s = key;
    std::transform(s.begin(), s.end(), s.begin(),
        [](unsigned char c) { return std::tolower(c); });

    d_ptr->m_stateValues[s] = value;
}

void Engine::setDefaultCursor(const int index, const std::string& cursor)
{
    d_ptr->m_defaultCursor[index] = cursor;
}

void Engine::playAnim(const std::string& animName)
{
    d_ptr->m_playingAnim.insert(animName); // Stores the current frame
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
    // TODO: cleanup this mess
    std::string fileName = d_ptr->m_dataPath + "video/" + movieFile;

    SDL_HideCursor();

    // Init libavcodec
    AVFormatContext* formatContext = avformat_alloc_context();
    if (avformat_open_input(&formatContext, fileName.c_str(), NULL, NULL) != 0) {
        std::cerr << "Unable to open file" << std::endl;
        SDL_ShowCursor();
        return;
    }

    if (avformat_find_stream_info(formatContext, NULL) < 0) {
        std::cerr << "Unable to find stream info" << std::endl;
        avformat_close_input(&formatContext);
        SDL_ShowCursor();
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
        std::cerr << "Unable to find video stream" << std::endl;
        avformat_close_input(&formatContext);
        SDL_ShowCursor();
        return;
    }

    AVCodecParameters* localCodecParametersVideo = formatContext->streams[videoStreamIndex]->codecpar;
    const AVCodec* localCodecVideo = avcodec_find_decoder(localCodecParametersVideo->codec_id);
    if (localCodecVideo == NULL) {
        std::cerr << "Unable to find codec" << std::endl;
        avformat_close_input(&formatContext);
        SDL_ShowCursor();
        return;
    }

    AVCodecContext* codecContextVideo = avcodec_alloc_context3(localCodecVideo);
    if (avcodec_parameters_to_context(codecContextVideo, localCodecParametersVideo) < 0) {
        std::cerr << "Unable to copy codec parameters" << std::endl;
        avformat_close_input(&formatContext);
        avcodec_free_context(&codecContextVideo);
        SDL_ShowCursor();
        return;
    }

    if (avcodec_open2(codecContextVideo, localCodecVideo, NULL) < 0) {
        std::cerr << "Unable to open codec" << std::endl;
        avformat_close_input(&formatContext);
        avcodec_free_context(&codecContextVideo);
        SDL_ShowCursor();
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
    SDL_AudioStream* streamAudio = nullptr;
    AVCodecParameters* localCodecParametersAudio = NULL;
    AVCodec* localCodecAudio = NULL;
    AVCodecContext* codecContextAudio = NULL;
    if (audioSreamIndex >= 0) {
        localCodecParametersAudio = formatContext->streams[audioSreamIndex]->codecpar;
        // TODO: supposed to be const
        localCodecAudio = (AVCodec*)avcodec_find_decoder(localCodecParametersAudio->codec_id);
        if (localCodecAudio == NULL) {
            std::cerr << "Unable to find codec" << std::endl;
            avformat_close_input(&formatContext);
            avcodec_free_context(&codecContextVideo);
            SDL_ShowCursor();
            return;
        }

        codecContextAudio = avcodec_alloc_context3(localCodecAudio);
        if (avcodec_parameters_to_context(codecContextAudio, localCodecParametersAudio) < 0) {
            std::cerr << "Unable to copy codec parameters" << std::endl;
            avformat_close_input(&formatContext);
            avcodec_free_context(&codecContextVideo);
            avcodec_free_context(&codecContextAudio);
            SDL_ShowCursor();
            return;
        }

        if (avcodec_open2(codecContextAudio, localCodecAudio, NULL) < 0) {
            std::cerr << "Unable to open codec" << std::endl;
            avformat_close_input(&formatContext);
            avcodec_free_context(&codecContextVideo);
            avcodec_free_context(&codecContextAudio);
            SDL_ShowCursor();
            return;
        }

        SDL_AudioSpec spec;
        SDL_zero(spec);
        spec.freq = codecContextAudio->sample_rate;
        spec.format = SDL_AUDIO_S16;
        spec.channels = codecContextAudio->ch_layout.nb_channels;

        SDL_AudioSpec dst = spec;
        streamAudio = SDL_CreateAudioStream(&spec, &dst);
        if (!streamAudio) {
            std::cerr << "SDL audio error: " << SDL_GetError() << std::endl;
            avformat_close_input(&formatContext);
            avcodec_free_context(&codecContextVideo);
            avcodec_free_context(&codecContextAudio);
            SDL_ShowCursor();
            return;
        }

        // Ouvrir le périphérique audio
        deviceId = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec);
        if (deviceId == 0) {
            std::cerr << "SDL audio error: " << SDL_GetError() << std::endl;
            avformat_close_input(&formatContext);
            avcodec_free_context(&codecContextVideo);
            avcodec_free_context(&codecContextAudio);
            SDL_DestroyAudioStream(streamAudio);
            SDL_ShowCursor();
            return;
        }
        SDL_BindAudioStream(deviceId, streamAudio);
    }

    // Decoding
    AVPacket* packet = av_packet_alloc();
    AVFrame* frame = av_frame_alloc();

    // Render video
    double frameRate = av_q2d(formatContext->streams[videoStreamIndex]->r_frame_rate);
    double waitTimeMs = 1000.0 / (double)frameRate;
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
                    SDL_PutAudioStreamData(
                        streamAudio,
                        frame->data[0],
                        frame->nb_samples * frame->ch_layout.nb_channels * sizeof(int16_t));
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

            struct SwsContext* swsCtx = sws_getContext(
                frame->width, frame->height, (AVPixelFormat)frame->format, // source
                frame->width, frame->height, AV_PIX_FMT_RGB565, // destination
                SWS_BILINEAR, NULL, NULL, NULL);

            if (!swsCtx) {
                std::cerr << "Failed to create swscale context" << std::endl;
                avformat_close_input(&formatContext);
                avcodec_free_context(&codecContextVideo);
                if (audioSreamIndex != 0) {
                    avcodec_free_context(&codecContextAudio);
                    SDL_DestroyAudioStream(streamAudio);
                }
                SDL_ShowCursor();
                return;
            }

            AVFrame* pFrameRGB565 = av_frame_alloc();
            pFrameRGB565->format = AV_PIX_FMT_RGB565;
            pFrameRGB565->width = frame->width;
            pFrameRGB565->height = frame->height;
            av_frame_get_buffer(pFrameRGB565, 0);

            sws_scale(swsCtx, frame->data, frame->linesize, 0, frame->height, pFrameRGB565->data, pFrameRGB565->linesize);

            std::vector<uint16_t> frameData(frame->width * frame->height);
            std::memcpy(frameData.data(), pFrameRGB565->data[0], frameData.size() * 2);
            d_ptr->m_rendererOgl.updateFrame(frameData.data());
            d_ptr->m_rendererOgl.renderFrame();
            SDL_GL_SwapWindow(d_ptr->m_window);

            // Cleanup
            sws_freeContext(swsCtx);
            av_frame_free(&pFrameRGB565);

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
        std::vector<EventManager::Event> events = d_ptr->m_event.getEvents();
        for (const EventManager::Event& event : events) {
            switch (event.type) {
            case EventManager::Event::Type::MouseClickLeft:
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
    avcodec_free_context(&codecContextVideo);
    if (audioSreamIndex != 0) {
        avcodec_free_context(&codecContextAudio);
        SDL_BindAudioStream(deviceId, streamAudio);
        SDL_DestroyAudioStream(streamAudio);
        SDL_CloseAudioDevice(deviceId);
    }
    avformat_close_input(&formatContext);

    SDL_ShowCursor();
}

void Engine::setAngle(const float pitch, const float yaw)
{
    d_ptr->m_pitch = pitch;
    d_ptr->m_yaw = yaw;
}

void Engine::fade(int start, int end, int timer)
{
    std::vector<uint16_t> imageDataBak = d_ptr->m_vrImageData;

    const std::chrono::milliseconds frameDelay(1000 / ENGINE_FPS);
    double duration = timer; // total fade duration in seconds
    double elapsed = 0.0;
    double currentFade = start;

    auto lastTime = std::chrono::high_resolution_clock::now();
    while (elapsed < duration) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        double delta = std::chrono::duration<double>(currentTime - lastTime).count();
        lastTime = currentTime;

        elapsed += delta;
        double t = std::clamp(elapsed / duration, 0.0, 1.0);

        // Linear interpolation
        currentFade = start + t * (end - start);

        for (uint16_t& pixel : d_ptr->m_vrImageData) {
            int r = (pixel >> 11) & 0x1F; // 5 bits
            int g = (pixel >> 5) & 0x3F; // 6 bits
            int b = pixel & 0x1F; // 5 bits

            r = std::clamp(static_cast<int>(r + currentFade), 0, 255);
            g = std::clamp(static_cast<int>(g + currentFade), 0, 255);
            b = std::clamp(static_cast<int>(b + currentFade), 0, 255);

            pixel = (r << 11) | (g << 5) | b;
        }
        d_ptr->render();
        d_ptr->m_vrImageData = imageDataBak;

        std::this_thread::sleep_for(frameDelay);

        // Update events
        std::vector<EventManager::Event> events = d_ptr->m_event.getEvents();
        for (const EventManager::Event& event : events) {
            switch (event.type) {
            case EventManager::Event::Type::MouseClickLeft:
                return;
            }
        }
    }
}

void Engine::whileLoop(int timer)
{
    std::vector<uint16_t> imageDataBak = d_ptr->m_vrImageData;

    const std::chrono::milliseconds frameDelay(1000 / ENGINE_FPS);
    double duration = timer; // Total fade duration in seconds
    double elapsed = 0.0;

    auto lastTime = std::chrono::high_resolution_clock::now();
    while (elapsed < duration) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        double delta = std::chrono::duration<double>(currentTime - lastTime).count();
        lastTime = currentTime;

        elapsed += delta;

        d_ptr->render();

        std::this_thread::sleep_for(frameDelay);

        // Update events
        std::vector<EventManager::Event> events = d_ptr->m_event.getEvents();
        for (const EventManager::Event& event : events) {
            switch (event.type) {
            case EventManager::Event::Type::MouseClickLeft:
                return;
            }
        }
    }
}

void Engine::untilLoop(const std::string& variable, const int value)
{
    std::vector<uint16_t> imageDataBak = d_ptr->m_vrImageData;

    const std::chrono::milliseconds frameDelay(1000 / ENGINE_FPS);
    double start = std::stod(this->getStateValue(variable));
    double elapsed = 0.0;

    auto lastTime = std::chrono::high_resolution_clock::now();
    while (start < value) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        double delta = std::chrono::duration<double>(currentTime - lastTime).count();
        lastTime = currentTime;

        start += delta;

        d_ptr->render();

        std::this_thread::sleep_for(frameDelay);

        // Update events
        std::vector<EventManager::Event> events = d_ptr->m_event.getEvents();
        for (const EventManager::Event& event : events) {
            switch (event.type) {
            case EventManager::Event::Type::MouseClickLeft:
                return;
            }
        }
    }
}

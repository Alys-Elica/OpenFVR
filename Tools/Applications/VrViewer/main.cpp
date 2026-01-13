#include <algorithm>
#include <chrono>
#include <iostream>
#include <thread>

#include <SDL3/SDL.h>

#include <ofnx/files/tst.h>
#include <ofnx/files/vr.h>
#include <ofnx/graphics/rendereropengl.h>
#include <ofnx/tools/log.h>

#define FNXVR_WINDOW_WIDTH 640
#define FNXVR_WINDOW_HEIGHT 480
#define FNXVR_WINDOW_FOV 1.0f
#define FNXVR_MOUSE_SENSITIVITY 0.1f
#define FNXVR_FPS 30.0
#define FNXVR_FRAMETIME 1000.0 / FNXVR_FPS

std::string removeExtension(const std::string& str)
{
    size_t lastindex = str.find_last_of(".");
    return str.substr(0, lastindex);
}

struct Event {
    enum Type {
        Quit,
        MainMenu,
        MouseClickLeft,
        MouseClickRight,
        MouseMove,
        MouseWheel,
    };

    Type type;
    float x = 0;
    float y = 0;
    float xRel = 0;
    float yRel = 0;
};

std::vector<Event> getEvents()
{
    std::vector<Event> eventList;

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            eventList.push_back({ Event::Quit });
        } else if (event.type == SDL_EVENT_KEY_DOWN) {
            if (event.key.key == SDLK_ESCAPE) {
                eventList.push_back({ Event::Quit });
            } else if (event.key.key == SDLK_RETURN) {
                eventList.push_back({ Event::MainMenu });
            }
        } else if (event.type == SDL_EVENT_MOUSE_MOTION) {
            eventList.push_back(Event { Event::MouseMove, event.motion.x, event.motion.y, event.motion.xrel, event.motion.yrel });
        } else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
            if (event.button.button == SDL_BUTTON_LEFT) {
                eventList.push_back({ Event::MouseClickLeft, event.button.x, event.button.y });
            } else if (event.button.button == SDL_BUTTON_RIGHT) {
                eventList.push_back({ Event::MouseClickRight, event.button.x, event.button.y });
            }
        } else if (event.type == SDL_EVENT_MOUSE_WHEEL) {
            eventList.push_back({ Event::MouseWheel, event.wheel.x, event.wheel.y });
        }
    }

    return eventList;
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        LOG_CRITICAL("Usage: {} <vr_file_vr>", argv[0]);
        return 1;
    }

    // Init SDL3
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        LOG_CRITICAL("SDL initialisation failed: {}", SDL_GetError());
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_Window* window = SDL_CreateWindow("FnxVR", FNXVR_WINDOW_WIDTH, FNXVR_WINDOW_HEIGHT, SDL_WINDOW_OPENGL);
    if (!window) {
        LOG_CRITICAL("Failed to create SDL window: {}", SDL_GetError());
        return false;
    }

    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    if (!glContext) {
        LOG_CRITICAL("Failed to create GL context: {}", SDL_GetError());
        SDL_DestroyWindow(window);
        return false;
    }

    SDL_SetWindowRelativeMouseMode(window, true);

    // Disable VSync
    SDL_GL_SetSwapInterval(0);

    const std::string vrFileName(argv[1]);

    LOG_INFO("Opening VR file {}", vrFileName);

    // Load VR file VR
    ofnx::files::Vr vrFile;
    if (!vrFile.load(vrFileName)) {
        LOG_CRITICAL("VR failed to load");
        return 1;
    }

    std::vector<uint16_t> imageData;
    if (!vrFile.getDataRgb565(imageData)) {
        LOG_CRITICAL("Failed to load VR image data");
        return 1;
    }

    // Load TST file if present
    std::string tstFileName = removeExtension(vrFileName) + ".tst";
    ofnx::files::Tst tstFile;
    if (tstFile.loadFile(tstFileName)) {
        LOG_CRITICAL("Corresponding TST file found");
    }

    // Init Ofnx manager
    ofnx::graphics::RendererOpenGL renderer;
    if (!renderer.init(FNXVR_WINDOW_WIDTH, FNXVR_WINDOW_HEIGHT, vrFile.getType() == ofnx::files::Vr::Type::VR2_STATIC_VR, (ofnx::graphics::RendererOpenGL::oglLoadFunc)SDL_GL_GetProcAddress)) {
        LOG_CRITICAL("Failed to init Ofnx manager");
        return 1;
    }

    // Main loop
    bool isVr = true;
    if (vrFile.getType() == ofnx::files::Vr::Type::VR_STATIC_VR
        || vrFile.getType() == ofnx::files::Vr::Type::VR2_STATIC_VR) {
        renderer.updateVr(imageData.data());
        isVr = true;
    } else {
        renderer.updateFrame(imageData.data());
        isVr = false;
    }

    bool isRunning = true;

    float yawDeg = 270.0f; // Horizontal (left/right)
    float pitchDeg = 90.0f; // Vertical (up/down) - 0.0f: down, 90.0f: straight, 180.0f: up
    float rollDeg = 0.0f; // Tilt (left/right)
    float fov = FNXVR_WINDOW_FOV;

    while (isRunning) {
        const std::chrono::time_point<std::chrono::steady_clock> start = std::chrono::steady_clock::now();

        // Handle events
        for (Event event : getEvents()) {
            switch (event.type) {
            case Event::Quit:
                isRunning = false;
                break;

            case Event::MouseMove: {
                yawDeg += event.xRel * FNXVR_MOUSE_SENSITIVITY;
                pitchDeg -= event.yRel * FNXVR_MOUSE_SENSITIVITY;

                if (yawDeg < 0.0f) {
                    yawDeg = 360.0f;
                } else if (yawDeg > 360.0f) {
                    yawDeg = 0.0f;
                }

                if (pitchDeg < 0.0f) {
                    pitchDeg = 0.0f;
                } else if (pitchDeg > 360.0f) {
                    pitchDeg = 0.0f;
                }

                pitchDeg = std::clamp(pitchDeg, 0.0f, 180.0f);

                // TST zone
                int tstZone = -1;
                if (isVr) {
                    tstZone = tstFile.checkZoneVr(yawDeg, pitchDeg);
                } else {
                    tstZone = tstFile.checkZoneStatic(event.x, event.y);
                }

                if (tstZone == -1) {
                    // renderer.setCursorSystem(ofnx::graphics::RendererOpenGL::CursorSystem::Default);
                } else {
                    // renderer.setCursorSystem(ofnx::graphics::RendererOpenGL::CursorSystem::Pointer);
                }
            } break;

            case Event::MouseWheel:
                fov -= event.y * 0.1f;
                fov = std::clamp(fov, 0.5f, 2.0f);
                break;

            case Event::MouseClickLeft:
                break;
            }
        }

        // Render
        if (isVr) {
            // vrFile.applyAnimationFrameRgb565("torche", imageData.data());
            renderer.updateVr(imageData.data());
            renderer.renderVr(FNXVR_WINDOW_WIDTH, FNXVR_WINDOW_HEIGHT, yawDeg, pitchDeg, rollDeg, fov);
        } else {
            renderer.renderFrame();
        }

        const std::chrono::time_point<std::chrono::steady_clock> end = std::chrono::steady_clock::now();
        double elapsedTime = (double)std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1000.0;

        if (FNXVR_FRAMETIME > elapsedTime) {
            std::this_thread::sleep_for(
                std::chrono::microseconds((int)((FNXVR_FRAMETIME - elapsedTime) * 1000.0)));
        }
    }

    // Clean up
    renderer.deinit();

    SDL_GL_DestroyContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

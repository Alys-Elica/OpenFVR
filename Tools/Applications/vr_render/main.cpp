#include <algorithm>
#include <chrono>
#include <cstring>
#include <iostream>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <ofnx/filetst.h>
#include <ofnx/filevr.h>
#include <ofnx/log.h>
#include <ofnx/vrrenderer.h>

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define WINDOW_FOV 1.0f
#define MOUSE_SENSITIVITY 0.1f

std::string removeExtension(const std::string& filename)
{
    std::size_t pos = filename.find_last_of('.');
    if (pos == std::string::npos) {
        return filename; // no extension
    }
    return filename.substr(0, pos);
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        LOG_ERROR("Usage: {} <vr>", argv[0]);
        return 1;
    }

    const std::string vrFileName(argv[1]);
    const std::string tstFileName(removeExtension(vrFileName) + ".tst");

    // Load VR file
    ofnx::FileVr vrFile;
    if (!vrFile.load(vrFileName)) {
        LOG_ERROR("VR failed to load");
        return 1;
    }

    if (vrFile.getType() != ofnx::FileVr::Type::VR_STATIC_VR && vrFile.getType() != ofnx::FileVr::Type::VR2_STATIC_VR) {
        LOG_ERROR("VR file is not panoramic");
        return 1;
    }

    std::vector<uint16_t> imageData;
    if (!vrFile.getDataRgb565(imageData)) {
        LOG_ERROR("Failed to load VR image data");
        return 1;
    }

    ofnx::FileTst tstFile;
    if (tstFile.load(tstFileName)) {
        LOG_INFO("TST file found");
    }

    // Init SDL
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        LOG_ERROR("SDL_Init failed - {}", SDL_GetError());
        return 1;
    }

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    if (!SDL_CreateWindowAndRenderer("fnx_vr", WINDOW_WIDTH, WINDOW_HEIGHT, 0, &window, &renderer)) {
        LOG_ERROR("SDL_CreateWindowAndRenderer failed - {}", SDL_GetError());
        SDL_Quit();
        return false;
    }

    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING, WINDOW_WIDTH, WINDOW_HEIGHT);
    if (!texture) {
        LOG_ERROR("Failed to create SDL texture - {}", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    void* texturePixels = nullptr;
    int texturePitch = 0;
    if (!SDL_LockTexture(texture, nullptr, &texturePixels, &texturePitch)) {
        LOG_ERROR("Failed to lock SDL texture - {}", SDL_GetError());
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    SDL_UnlockTexture(texture);

    ofnx::VrRenderer vr;
    vr.init(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, texturePitch / sizeof(uint16_t), vrFile.getType() == ofnx::FileVr::Type::VR2_STATIC_VR);
    std::memcpy(vr.cubemapBuffer(), imageData.data(), imageData.size() * sizeof(uint16_t));

    SDL_SetWindowRelativeMouseMode(window, true);

    // Main loop
    bool isRunning = true;

    float yawDeg = 270.0f; // Horizontal (left/right)
    float pitchDeg = 90.0f; // Vertical (up/down) - 0.0f: down, 1.5: straight, 3.0f: up
    float rollDeg = 0.0f; // Tilt (left/right)
    float fov = WINDOW_FOV;
    auto lastStats = std::chrono::steady_clock::now();
    int frameCount = 0;
    std::chrono::microseconds renderTimeSum(0);

    while (isRunning) {
        // Handle events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                isRunning = false;
            } else if (event.type == SDL_EVENT_KEY_DOWN) {
                if (event.key.key == SDLK_ESCAPE) {
                    isRunning = false;
                } else if (event.key.key == SDLK_Q) {
                    rollDeg -= 0.5f;
                } else if (event.key.key == SDLK_E) {
                    rollDeg += 0.5f;
                }
            } else if (event.type == SDL_EVENT_MOUSE_MOTION) {
                yawDeg += event.motion.xrel * MOUSE_SENSITIVITY;
                pitchDeg -= event.motion.yrel * MOUSE_SENSITIVITY;

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
            } else if (event.type == SDL_EVENT_MOUSE_WHEEL) {
                fov -= event.wheel.y * 0.1f;
                fov = std::clamp(fov, 0.5f, 2.0f);
            }
        }

        // Render directly into the streaming texture to avoid a second full-frame copy.
        if (!SDL_LockTexture(texture, nullptr, &texturePixels, &texturePitch)) {
            LOG_ERROR("Failed to lock SDL texture - {}", SDL_GetError());
            break;
        }

        const std::chrono::time_point<std::chrono::steady_clock> start
            = std::chrono::steady_clock::now();
        vr.render(
            static_cast<uint16_t*>(texturePixels),
            yawDeg * 0.0174532925f - 1.570795f, // Rotate 90 degrees
            pitchDeg * 0.0174532925f,
            rollDeg * 0.0174532925f,
            fov);
        const std::chrono::time_point<std::chrono::steady_clock> end = std::chrono::steady_clock::now();
        renderTimeSum += std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        frameCount++;

        SDL_UnlockTexture(texture);

        const auto now = std::chrono::steady_clock::now();
        if (now - lastStats >= std::chrono::seconds(1)) {
            LOG_INFO("VR render time avg: {}ms", (renderTimeSum.count() / frameCount) / 1000.0);
            renderTimeSum = std::chrono::microseconds(0);
            frameCount = 0;
            lastStats = now;
        }

        // Draw to renderer
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Render the scene
        SDL_RenderTexture(renderer, texture, nullptr, nullptr);

        // Draw cursor
        if (tstFile.checkZoneVr(yawDeg, pitchDeg) != -1) {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        } else {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        }
        SDL_RenderLine(renderer, WINDOW_WIDTH / 2 - 10, WINDOW_HEIGHT / 2, WINDOW_WIDTH / 2 + 10, WINDOW_HEIGHT / 2);
        SDL_RenderLine(renderer, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 - 10, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 + 10);

        SDL_RenderPresent(renderer);
    }

    // Cleaning
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

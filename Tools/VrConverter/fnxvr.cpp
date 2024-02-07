#include "fnxvr.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <vector>

#include <SDL.h>
#include <fvr/file.h>
#include <fvrrenderer.h>

#include "fvr_files/fvr_vr.h"
#include "fvr_files/fvr_tst.h"

// Private implementation
class FnxVrPrivate
{
    friend class FnxVr;

private:
    FvrVr m_vrFile;
    FvrTst m_tstFile;

    FvrRenderer m_renderer;
};

// Public implementation
#define FNXVR_WINDOW_TITLE "FnxVR"
#define FNXVR_WINDOW_WIDTH 1024
#define FNXVR_WINDOW_HEIGHT 768
#define FNXVR_WINDOW_FOV 1.0f
#define FNXVR_FPS 60.0f
#define FNXVR_MOUSE_SENSITIVITY 0.1f

FnxVr::FnxVr()
{
    d_ptr = new FnxVrPrivate();

    if (!isValid())
    {
        return;
    }

    d_ptr->m_renderer.setResolution(FNXVR_WINDOW_WIDTH, FNXVR_WINDOW_HEIGHT);
}

FnxVr::~FnxVr()
{
    delete d_ptr;
}

bool FnxVr::isValid()
{
    return true;
}

bool FnxVr::loadFile(const std::string &vrFileName)
{
    if (!d_ptr->m_vrFile.open(vrFileName))
    {
        return false;
    }
    if (!d_ptr->m_vrFile.isOpen())
    {
        std::cerr << "VR file is not open" << std::endl;
        return false;
    }

    if (d_ptr->m_vrFile.getType() != FvrVr::Type::VR_STATIC_VR)
    {
        std::cerr << "VR file is not panoramic" << std::endl;
        return false;
    }

    Image vrImage;
    d_ptr->m_vrFile.getImage(vrImage);
    if (!vrImage.isValid())
    {
        std::cerr << "VR image is not valid" << std::endl;
        return false;
    }

    std::vector<uint8_t> imageData;
    if (!vrImage.toRgb565(imageData))
    {
        std::cerr << "Failed to convert VR image to RGB565" << std::endl;
        return false;
    }

    unsigned short *buffer = d_ptr->m_renderer.cubemapBuffer();
    std::memcpy(buffer, imageData.data(), imageData.size());

    return true;
}

bool FnxVr::loadTstFile(const std::string &tstFileName)
{
    return d_ptr->m_tstFile.loadFile(tstFileName);
}

bool FnxVr::loop()
{
    // Init SDL
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow(
        FNXVR_WINDOW_TITLE,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        FNXVR_WINDOW_WIDTH, FNXVR_WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN | SDL_WINDOW_MOUSE_CAPTURE | SDL_WINDOW_INPUT_GRABBED | SDL_WINDOW_MOUSE_FOCUS);

    if (!window)
    {
        std::cerr << "Failed to create SDL window" << std::endl;
        return false;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        std::cerr << "Failed to create SDL renderer" << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return false;
    }

    // SDL surface
    SDL_Surface *surface = SDL_CreateRGBSurface(
        0,
        FNXVR_WINDOW_WIDTH, FNXVR_WINDOW_HEIGHT,
        16,
        0x0000F800, 0x000007E0, 0x0000001F, 0x00000000);
    if (!surface)
    {
        std::cerr << "Failed to create SDL surface" << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return false;
    }

    SDL_ShowCursor(SDL_DISABLE);
    SDL_SetRelativeMouseMode(SDL_TRUE);

    // Main loop
    uint64_t tickBegin = SDL_GetTicks64();
    bool isRunning = true;

    float yawDeg = 270.0f;  // Horizontal (left/right)
    float pitchDeg = 90.0f; // Vertical (up/down) - 0.0f: down, 1.5: straight, 3.0f: up
    float rollDeg = 0.0f;   // Tilt (left/right)
    float fov = FNXVR_WINDOW_FOV;

    while (isRunning)
    {
        // Handle events
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                isRunning = false;
            }
            else if (event.type == SDL_KEYDOWN)
            {
                if (event.key.keysym.sym == SDLK_ESCAPE)
                {
                    isRunning = false;
                }
            }
            else if (event.type == SDL_MOUSEMOTION)
            {
                yawDeg += event.motion.xrel * FNXVR_MOUSE_SENSITIVITY;
                pitchDeg -= event.motion.yrel * FNXVR_MOUSE_SENSITIVITY;

                if (yawDeg < 0.0f)
                {
                    yawDeg = 360.0f;
                }
                else if (yawDeg > 360.0f)
                {
                    yawDeg = 0.0f;
                }

                if (pitchDeg < 0.0f)
                {
                    pitchDeg = 0.0f;
                }
                else if (pitchDeg > 360.0f)
                {
                    pitchDeg = 0.0f;
                }

                pitchDeg = std::clamp(pitchDeg, 0.0f, 180.0f);
            }
            else if (event.type == SDL_MOUSEWHEEL)
            {
                fov -= event.wheel.y * 0.1f;
                fov = std::clamp(fov, 0.5f, 2.0f);
            }
        }

        // Render
        float yawRad = yawDeg * 0.0174532925f;     // Convert to radians
        float pitchRad = pitchDeg * 0.0174532925f; // Convert to radians
        float rollRad = rollDeg * 0.0174532925f;   // Convert to radiansyawRad,

        // Draw to surface
        SDL_LockSurface(surface);
        d_ptr->m_renderer.render(
            (unsigned short *)surface->pixels,
            yawRad - 1.570795f, // Rotate 90 degrees
            pitchRad,
            rollRad,
            fov);

        SDL_UnlockSurface(surface);

        // Draw to renderer
        SDL_RenderClear(renderer);

        // Render the scene
        SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_DestroyTexture(texture);

        // Draw cursor
        if (d_ptr->m_tstFile.checkAngleZone(yawRad, pitchRad) != -1)
        {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        }
        else
        {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        }

        SDL_RenderDrawLine(renderer, FNXVR_WINDOW_WIDTH / 2 - 10, FNXVR_WINDOW_HEIGHT / 2, FNXVR_WINDOW_WIDTH / 2 + 10, FNXVR_WINDOW_HEIGHT / 2);
        SDL_RenderDrawLine(renderer, FNXVR_WINDOW_WIDTH / 2, FNXVR_WINDOW_HEIGHT / 2 - 10, FNXVR_WINDOW_WIDTH / 2, FNXVR_WINDOW_HEIGHT / 2 + 10);

        SDL_RenderPresent(renderer);
    }

    // Clean up
    SDL_FreeSurface(surface);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return true;
}

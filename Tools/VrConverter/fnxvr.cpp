#include "fnxvr.h"

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <vector>
#include <cstring>

#include <windows.h>

#include <SDL.h>

#include "fvr_files/fvr_vr.h"

// Private implementation
class FnxVrPrivate
{
    friend class FnxVr;

public:
    FnxVrPrivate()
    {
        libFvr = LoadLibrary(TEXT("Fnx_vr.dll"));
        if (!libFvr)
        {
            std::cerr << "Failed to load Fnx_vr.dll" << std::endl;
            return;
        }

        // Unpack image functions
        FVR_init = (FVR_cdtor)GetProcAddress(libFvr, "??0FNX_VR@@QAE@XZ");
        FVR_deinit = (FVR_cdtor)GetProcAddress(libFvr, "??1FNX_VR@@QAE@XZ");
        if (!FVR_init || !FVR_deinit)
        {
            std::cerr << "Failed to get DCT const/dest function pointer" << std::endl;
            clean();
            return;
        }

        FVR_Buffer = (FVR_Buffer_t)GetProcAddress(libFvr, "?Buffer@FNX_VR@@QAEPAGXZ");
        FVR_Draw = (FVR_Draw_t)GetProcAddress(libFvr, "?Draw@FNX_VR@@QAEXPAGMMMM@Z");
        FVR_Init_Resolution = (FVR_Init_Resolution_t)GetProcAddress(libFvr, "?Init_Resolution@FNX_VR@@QAEHMMMMHM@Z");
        if (!FVR_Buffer || !FVR_Draw || !FVR_Init_Resolution)
        {
            std::cerr << "Failed to get function pointer" << std::endl;
            clean();
            return;
        }

        // DCT object size
        m_fvrObj = static_cast<uint8_t *>(malloc(1212));

        FVR_init(m_fvrObj);
    };
    ~FnxVrPrivate()
    {
        delete[] m_fvrObj;
    };

    void clean()
    {
        if (m_fvrObj)
        {
            if (FVR_deinit)
            {
                FVR_deinit(m_fvrObj);
            }
            free(m_fvrObj);
        }

        m_fvrObj = NULL;

        FreeLibrary(libFvr);
        libFvr = NULL;

        FVR_init = NULL;
        FVR_deinit = NULL;
    }

private:
    typedef void(__thiscall *FVR_cdtor)(void *thisptr);
    typedef unsigned short *(__thiscall *FVR_Buffer_t)(void *thisptr);
    typedef void(__thiscall *FVR_Draw_t)(
        void *thisptr,
        unsigned short *outputBuffer, // 16bit RGB565 buffer
        float yaw,                    // VR projection yaw (left/right) in radians
        float pitch,                  // VR projection pitch (up/down) in radians
        float roll,                   // VR projection roll (tilt) in radians
        float fov);
    typedef int(__thiscall *FVR_Init_Resolution_t)(
        void *thisptr,
        float xOffset, // VR projection x offset
        float yOffset, // VR projection y offset
        float width,   // VR projection width
        float height,  // VR projection height
        int stride,    // VR projection stride
        float param_6);

private:
    HINSTANCE libFvr = NULL;

    // Function pointers
    FVR_cdtor FVR_init = NULL;                        // FVR constructor
    FVR_cdtor FVR_deinit = NULL;                      // FVR destructor
    FVR_Buffer_t FVR_Buffer = NULL;                   // Returns the 16bit RGB565 VR image data buffer
    FVR_Draw_t FVR_Draw = NULL;                       // Draws VR projection to the given 16bit RGB565 buffer
    FVR_Init_Resolution_t FVR_Init_Resolution = NULL; // Initializes the VR projection resolution

    uint8_t *m_fvrObj; // FVR object

    FvrVr m_vrFile;
};

// Public implementation
#define FNXVR_WINDOW_TITLE "FnxVR"
#define FNXVR_WINDOW_WIDTH 1024
#define FNXVR_WINDOW_HEIGHT 768
#define FNXVR_WINDOW_FOV 1.0f
#define FNXVR_FPS 60.0
#define FNXVR_MOUSE_SENSITIVITY 1.5f

FnxVr::FnxVr()
{
    d_ptr = new FnxVrPrivate();

    if (!isValid())
    {
        return;
    }

    d_ptr->FVR_Init_Resolution(
        d_ptr->m_fvrObj,
        0.0f, 0.0f,
        FNXVR_WINDOW_WIDTH, FNXVR_WINDOW_HEIGHT,
        FNXVR_WINDOW_WIDTH, 1.0f);
}

FnxVr::~FnxVr()
{
    delete d_ptr;
}

bool FnxVr::isValid()
{
    return d_ptr->libFvr != NULL;
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

    unsigned short *buffer = d_ptr->FVR_Buffer(d_ptr->m_fvrObj);
    std::memcpy(buffer, imageData.data(), imageData.size());

    return true;
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
    uint64_t ticksA = SDL_GetTicks64();
    bool isRunning = true;

    float yawDeg = 270.0f;  // Horizontal (left/right)
    float pitchDeg = 90.0f; // Vertical (up/down) - 0.0f: down, 1.5: straight, 3.0f: up
    float rollDeg = 0.0f;   // Tilt (left/right)

    while (isRunning)
    {
        // Delta thingy
        uint64_t ticksB = SDL_GetTicks64();
        uint64_t deltaMs = ticksB - ticksA;
        float delta = deltaMs / 1000.0f;

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
                yawDeg += event.motion.xrel * FNXVR_MOUSE_SENSITIVITY * delta;
                pitchDeg -= event.motion.yrel * FNXVR_MOUSE_SENSITIVITY * delta;

                if (yawDeg < 0.0f)
                {
                    yawDeg = 360.0f;
                }
                else if (pitchDeg > 360.0f)
                {
                    pitchDeg = 0.0f;
                }

                pitchDeg = std::clamp(pitchDeg, 0.0f, 180.0f);
            }
        }

        // FPS limit
        if (deltaMs < 1000.0 / FNXVR_FPS)
        {
            SDL_Delay(10);
            continue;
        }

        ticksA = ticksB;

        // Render
        // Draw to surface
        SDL_LockSurface(surface);
        d_ptr->FVR_Draw(
            d_ptr->m_fvrObj,
            (unsigned short *)surface->pixels,
            yawDeg * 0.0174532925f,   // Convert to radians
            pitchDeg * 0.0174532925f, // Convert to radians
            rollDeg * 0.0174532925f,  // Convert to radians
            FNXVR_WINDOW_FOV);
        SDL_UnlockSurface(surface);

        // Draw to renderer
        SDL_RenderClear(renderer);
        SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_DestroyTexture(texture);
        SDL_RenderPresent(renderer);
    }

    // Clean up
    SDL_FreeSurface(surface);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return true;
}

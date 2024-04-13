#include "render.h"

#include <cstring>
#include <iostream>

#include <SDL.h>
#include <SDL_image.h>

/* Constants */
#define WINDOW_TITLE "FvrEnginePoC"
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

#define IMAGE_DIR "data/image/"

/* Private */
class Render::RenderPrivate {
    friend class Render;

private:
    bool m_isInit = false;

    SDL_Window* m_window = NULL;
    SDL_Renderer* m_renderer = NULL;
    SDL_Surface* m_surface = NULL;
};

/* Public */
Render::Render()
{
    d_ptr = new RenderPrivate();
}

Render::~Render()
{
    delete d_ptr;
}

bool Render::init()
{
    d_ptr->m_window = SDL_CreateWindow(
        WINDOW_TITLE,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN | SDL_WINDOW_MOUSE_CAPTURE | SDL_WINDOW_INPUT_GRABBED | SDL_WINDOW_MOUSE_FOCUS);

    if (!d_ptr->m_window) {
        std::cerr << "Failed to create SDL window" << std::endl;
        return false;
    }

    d_ptr->m_renderer = SDL_CreateRenderer(
        d_ptr->m_window,
        -1,
        SDL_RENDERER_ACCELERATED);
    if (!d_ptr->m_renderer) {
        std::cerr << "Failed to create SDL renderer" << std::endl;
        deinit();
        return false;
    }

    // SDL surface
    d_ptr->m_surface = SDL_CreateRGBSurface(
        0,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        16,
        0x0000F800, 0x000007E0, 0x0000001F, 0x00000000);
    if (!d_ptr->m_surface) {
        std::cerr << "Failed to create SDL surface" << std::endl;
        deinit();
        return false;
    }

    d_ptr->m_isInit = true;

    return true;
}

void Render::render(Engine& engine)
{
    if (engine.isPanoramic()) {
        SDL_SetRelativeMouseMode(SDL_TRUE);
        SDL_WarpMouseInWindow(d_ptr->m_window, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
    } else {
        SDL_SetRelativeMouseMode(SDL_FALSE);
    }

    // Draw to renderer
    SDL_RenderClear(d_ptr->m_renderer);

    // Copy frame buffer to surface
    std::vector<uint16_t>& frameBuffer = engine.getFrameBuffer();
    std::memcpy(d_ptr->m_surface->pixels, frameBuffer.data(), frameBuffer.size() * sizeof(uint16_t));

    // Render the scene
    SDL_Texture* texture = SDL_CreateTextureFromSurface(d_ptr->m_renderer, d_ptr->m_surface);
    SDL_RenderCopy(d_ptr->m_renderer, texture, NULL, NULL);
    SDL_DestroyTexture(texture);

    // Draw cursor
    SDL_Surface* curImage = nullptr;

    if (engine.isOnZone()) {
        // Specific zone
        SDL_SetRenderDrawColor(d_ptr->m_renderer, 255, 0, 0, 255);

        // Draw cursor image
        std::string cursorFile = engine.getWarpZoneCursor(engine.pointedZone());
        if (!cursorFile.empty()) {
            curImage = IMG_Load((IMAGE_DIR + cursorFile).c_str());
        } else {
            curImage = IMG_Load(IMAGE_DIR "cursor2.gif");
        }
    } else {
        // No specific zone
        SDL_SetRenderDrawColor(d_ptr->m_renderer, 255, 255, 255, 255);

        curImage = IMG_Load(IMAGE_DIR "cursor1.gif");
    }

    if (curImage) {
        // Remove black background
        SDL_SetColorKey(curImage, SDL_TRUE, SDL_MapRGB(curImage->format, 0, 0, 0));

        if (engine.isPanoramic()) {
            // Draw cursor image
            SDL_Texture* curTexture = SDL_CreateTextureFromSurface(d_ptr->m_renderer, curImage);
            SDL_Rect curRect = { WINDOW_WIDTH / 2 - curImage->w / 2, WINDOW_HEIGHT / 2 - curImage->h / 2, curImage->w, curImage->h };
            SDL_RenderCopy(d_ptr->m_renderer, curTexture, NULL, &curRect);
            SDL_DestroyTexture(curTexture);
        } else {
            SDL_Cursor* cursor = SDL_CreateColorCursor(curImage, curImage->w / 2, curImage->h / 2);
            SDL_SetCursor(cursor);
        }

        SDL_FreeSurface(curImage);
    }

    SDL_RenderPresent(d_ptr->m_renderer);
}

void Render::deinit()
{
    if (d_ptr->m_surface != NULL) {
        SDL_FreeSurface(d_ptr->m_surface);
    }
    if (d_ptr->m_renderer != NULL) {
        SDL_DestroyRenderer(d_ptr->m_renderer);
    }
    if (d_ptr->m_window != NULL) {
        SDL_DestroyWindow(d_ptr->m_window);
    }

    d_ptr->m_isInit = false;
}

int Render::getWidth() const
{
    return WINDOW_WIDTH;
}

int Render::getHeight() const
{
    return WINDOW_HEIGHT;
}

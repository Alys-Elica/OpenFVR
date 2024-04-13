#include "input.h"

#include <SDL.h>

/* Private */
class Input::InputPrivate {
    friend class Input;

private:
    bool m_isInit = false;
};

/* Public */
Input::Input()
{
    d_ptr = new InputPrivate();
}

Input::~Input()
{
    delete d_ptr;
}

bool Input::init()
{
    d_ptr->m_isInit = true;
    return true;
}

std::vector<Input::Event> Input::update()
{
    std::vector<Input::Event> eventList;

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            eventList.push_back({ Event::Quit });
        } else if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                eventList.push_back({ Event::Quit });
            } else if (event.key.keysym.sym == SDLK_RETURN) {
                eventList.push_back({ Event::MainMenu });
            }
        } else if (event.type == SDL_MOUSEMOTION) {
            eventList.push_back({ Event::MouseMove, event.motion.x, event.motion.y, event.motion.xrel, event.motion.yrel });
        } else if (event.type == SDL_MOUSEBUTTONDOWN) {
            if (event.button.button == SDL_BUTTON_LEFT) {
                eventList.push_back({ Event::MouseClickLeft, event.button.x, event.button.y });
            } else if (event.button.button == SDL_BUTTON_RIGHT) {
                eventList.push_back({ Event::MouseClickRight, event.button.x, event.button.y });
            }
        }
    }

    return eventList;
}

void Input::deinit()
{
    if (!d_ptr->m_isInit) {
        return;
    }

    d_ptr->m_isInit = false;
}

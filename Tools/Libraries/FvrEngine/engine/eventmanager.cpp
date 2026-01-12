#include "eventmanager.h"

#include <SDL3/SDL.h>

EventManager::EventManager()
{
}

EventManager::~EventManager()
{
}

bool EventManager::init()
{
    return true;
}

void EventManager::deinit()
{
}

std::vector<EventManager::Event> EventManager::getEvents()
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

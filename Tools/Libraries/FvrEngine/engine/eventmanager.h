#ifndef ENGINE_EVENTMANAGER_H
#define ENGINE_EVENTMANAGER_H

#include <vector>

class EventManager {
public:
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

public:
    EventManager();
    ~EventManager();

    bool init();
    void deinit();

    std::vector<Event> getEvents();
};

#endif // ENGINE_EVENTMANAGER_H

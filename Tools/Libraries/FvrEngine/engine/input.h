#ifndef INPUT_H
#define INPUT_H

#include <vector>

class Input {
public:
    struct Event {
        enum Type {
            Quit,
            MainMenu,
            MouseClickLeft,
            MouseClickRight,
            MouseMove,
        };

        Type type;
        int x = 0;
        int y = 0;
        int xRel = 0;
        int yRel = 0;
    };

public:
    Input();
    ~Input();

    bool init();
    std::vector<Event> update();
    void deinit();

private:
    class InputPrivate;
    InputPrivate* d_ptr;
};

#endif // INPUT_H

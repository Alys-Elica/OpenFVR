#include <iostream>

#include <engine.h>

#include "pluginlouvre.h"

int main(int argc, char* argv[])
{
    Engine engine;
    if (!engine.init()) {
        std::cerr << "Failed to initialize engine" << std::endl;
        return 1;
    }

    registerPluginLouvre(engine);

    engine.loop();
    engine.deinit();

    return 0;
}

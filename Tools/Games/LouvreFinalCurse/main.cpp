#include <iostream>

#include <engine.h>
#include <ofnx/tools/log.h>

#include "pluginlouvre.h"

int main(int argc, char* argv[])
{
    Engine engine;
    if (!engine.init()) {
        LOG_CRITICAL("Failed to initialize engine");
        return 1;
    }

    registerPluginLouvre(engine);

    engine.loop();
    engine.deinit();

    return 0;
}

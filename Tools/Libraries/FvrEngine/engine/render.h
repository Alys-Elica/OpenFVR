#ifndef RENDER_H
#define RENDER_H

#include <cstdint>
#include <vector>

#include <fvr_files/fvr_vr.h>

#include "engine.h"

class Render {
public:
    Render();
    ~Render();

    bool init();
    void render(Engine& engine);
    void deinit();

    int getWidth() const;
    int getHeight() const;

private:
    class RenderPrivate;
    RenderPrivate* d_ptr;
};

#endif // RENDER_H

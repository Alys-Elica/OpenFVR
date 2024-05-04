#ifndef ENGINE_H
#define ENGINE_H

#include "libfvrengine_globals.h"

#include <cstdint>
#include <functional>
#include <string>
#include <variant>
#include <vector>

#include <fvr_files/fvr_script.h>

class LIBFVRENGINE_EXPORT Engine {
public:
    using ScriptFunction = std::function<void(Engine& engine, std::vector<FvrScript::InstructionParam> args)>;

public:
    Engine();
    ~Engine();

    bool init();
    void loop();
    void deinit();

    void registerScriptFunction(const std::string& name, const ScriptFunction& function);

    bool inMovieMode() const;
    bool isPanoramic() const;
    bool isOnZone() const;
    int pointedZone() const;
    std::vector<uint16_t>& getFrameBuffer();

    void registerKeyWarp(int key, const std::string& warpName);
    void unregisterKeyWarp(int key);
    void clearKeyWarps();

    void setWarpZoneCursor(const std::string& warp, int zoneId, const std::string& cursorFile);
    std::string getWarpZoneCursor(int zoneId) const;

    void end();

    void gotoWarp(const std::string& warpName);

    FvrScript::InstructionParam getStateValue(const std::string& key);
    void setStateValue(const std::string& key, const FvrScript::InstructionParam& value);

    void playAnim(const std::string& animName);

    void playSound(const std::string& soundFile, uint8_t volume, bool loop = false);
    void stopSound(const std::string& soundFile);

    void playMovie(const std::string& movieFile);

private:
    class EnginePrivate;
    EnginePrivate* d_ptr;
};

#endif // ENGINE_H

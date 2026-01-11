#include "script.h"

#include <algorithm>
#include <iostream>

#include "engine.h"

void fvrGotoWarp(Engine& engine, std::vector<ofnx::files::Lst::InstructionParam> args)
{
    if (args.size() != 1) {
        std::cerr << "fvrGotoWarp: invalid argument count" << std::endl;
        return;
    }

    if (!std::holds_alternative<std::string>(args[0])) {
        std::cerr << "fvrGotoWarp: invalid argument type" << std::endl;
        return;
    }

    std::string warpId = std::get<std::string>(args[0]);

    engine.gotoWarp(warpId);
}

void fvrPlaySound(Engine& engine, std::vector<ofnx::files::Lst::InstructionParam> args)
{
    if (args.size() != 3) {
        std::cerr << "fvrPlaySound: invalid argument count" << std::endl;
        return;
    }

    if (!std::holds_alternative<std::string>(args[0]) || !std::holds_alternative<double>(args[1]) || !std::holds_alternative<double>(args[2])) {
        std::cerr << "fvrPlaySound: invalid argument type" << std::endl;
        return;
    }

    std::string sound = std::get<std::string>(args[0]);
    double volume = std::get<double>(args[1]);
    double loop = std::get<double>(args[2]);

    // To lowercase
    std::transform(sound.begin(), sound.end(), sound.begin(), ::tolower);

    engine.playSound(sound, (int)volume, loop == -1);
}

void fvrStopSound(Engine& engine, std::vector<ofnx::files::Lst::InstructionParam> args)
{
    if (args.size() != 1) {
        std::cerr << "fvrStopSound: invalid argument count" << std::endl;
        return;
    }

    if (!std::holds_alternative<std::string>(args[0])) {
        std::cerr << "fvrStopSound: invalid argument type" << std::endl;
        return;
    }

    std::string sound = std::get<std::string>(args[0]);

    // To lowercase
    std::transform(sound.begin(), sound.end(), sound.begin(), ::tolower);

    engine.stopSound(sound);
}

void fvrPlaySound3d(Engine& engine, std::vector<ofnx::files::Lst::InstructionParam> args)
{
    if (args.size() != 4) {
        std::cerr << "fvrPlaySound3d: invalid argument count" << std::endl;
        return;
    }

    if (!std::holds_alternative<std::string>(args[0]) || !std::holds_alternative<double>(args[1]) || !std::holds_alternative<double>(args[2]) || !std::holds_alternative<double>(args[3])) {
        std::cerr << "fvrPlaySound3d: invalid argument type" << std::endl;
        return;
    }

    std::string sound = std::get<std::string>(args[0]);
    double x = std::get<double>(args[1]);
    double y = std::get<double>(args[2]);
    double z = std::get<double>(args[3]);

    // TODO: implement properly
    engine.playSound(sound, 100, false);
}

void fvrStopSound3d(Engine& engine, std::vector<ofnx::files::Lst::InstructionParam> args)
{
    if (args.size() != 1) {
        std::cerr << "fvrStopSound3d: invalid argument count" << std::endl;
        return;
    }

    if (!std::holds_alternative<std::string>(args[0])) {
        std::cerr << "fvrStopSound3d: invalid argument type" << std::endl;
        return;
    }

    std::string sound = std::get<std::string>(args[0]);

    // TODO: implement properly
    engine.stopSound(sound);
}

void fvrPlayMusic(Engine& engine, std::vector<ofnx::files::Lst::InstructionParam> args)
{
    if (args.size() != 1) {
        std::cerr << "fvrPlayMusic: invalid argument count" << std::endl;
        return;
    }

    if (!std::holds_alternative<std::string>(args[0])) {
        std::cerr << "fvrPlayMusic: invalid argument type" << std::endl;
        return;
    }

    std::string music = std::get<std::string>(args[0]);

    // TODO: implement properly
    engine.playSound(music, 100, true);
}

void fvrStopMusic(Engine& engine, std::vector<ofnx::files::Lst::InstructionParam> args)
{
    if (args.size() != 1) {
        std::cerr << "fvrStopMusic: invalid argument count" << std::endl;
        return;
    }

    if (!std::holds_alternative<std::string>(args[0])) {
        std::cerr << "fvrStopMusic: invalid argument type" << std::endl;
        return;
    }

    std::string music = std::get<std::string>(args[0]);

    engine.stopSound(music);
}

void fvrSet(Engine& engine, std::vector<ofnx::files::Lst::InstructionParam> args)
{
    if (args.size() != 2) {
        std::cerr << "fvrSet: invalid argument count" << std::endl;
        return;
    }

    if (!std::holds_alternative<std::string>(args[0]) || !std::holds_alternative<double>(args[1])) {
        std::cerr << "fvrSet: invalid argument type" << std::endl;
        return;
    }

    std::string flag = std::get<std::string>(args[0]);
    double value = std::get<double>(args[1]);

    engine.setStateValue(flag, value);
}

void fvrLockKey(Engine& engine, std::vector<ofnx::files::Lst::InstructionParam> args)
{
    if (args.size() != 2) {
        std::cerr << "fvrLockKey: invalid argument count" << std::endl;
        return;
    }

    if (!std::holds_alternative<double>(args[0]) || (!std::holds_alternative<double>(args[1]) && !std::holds_alternative<std::string>(args[1]))) {
        std::cerr << "fvrLockKey: invalid argument type" << std::endl;
        return;
    }

    /*
     * 0: Esc
     * 12: Right-click
     */
    double key = std::get<double>(args[0]);

    // TODO: better way to handle this ?
    try {
        if (std::get<double>(args[1]) == 0) {
            engine.unregisterKeyWarp((int)key);
        }
    } catch (const std::exception&) {
        std::string warp = std::get<std::string>(args[1]);
        // Remove trailing '.vr' from warp name
        // warp = warp.substr(0, warp.size() - 3);

        // To lowercase
        std::transform(warp.begin(), warp.end(), warp.begin(), ::tolower);

        engine.registerKeyWarp((int)key, warp);
    }
}

void fvrResetLockKey(Engine& engine, std::vector<ofnx::files::Lst::InstructionParam> args)
{
    engine.clearKeyWarps();
}

void fvrSetCursor(Engine& engine, std::vector<ofnx::files::Lst::InstructionParam> args)
{
    if (args.size() != 3) {
        std::cerr << "fvrSetCursor: invalid argument count" << std::endl;
        return;
    }

    if (!std::holds_alternative<std::string>(args[0]) || !std::holds_alternative<std::string>(args[1]) || !std::holds_alternative<double>(args[2])) {
        std::cerr << "fvrSetCursor: invalid argument type" << std::endl;
        return;
    }

    std::string cursor = std::get<std::string>(args[0]);
    std::string warp = std::get<std::string>(args[1]);
    double zone = std::get<double>(args[2]);

    // To lowercase
    std::transform(cursor.begin(), cursor.end(), cursor.begin(), ::tolower);
    std::transform(warp.begin(), warp.end(), warp.begin(), ::tolower);

    engine.setWarpZoneCursor(warp, (int)zone, cursor);
}

void fvrSetCursorDefault(Engine& engine, std::vector<ofnx::files::Lst::InstructionParam> args)
{
    if (args.size() != 2) {
        std::cerr << "fvrSetCursorDefault: invalid argument count" << std::endl;
        return;
    }

    if (!std::holds_alternative<double>(args[0]) || !std::holds_alternative<std::string>(args[1])) {
        std::cerr << "fvrSetCursorDefault: invalid argument type" << std::endl;
        return;
    }

    double value = std::get<double>(args[0]);
    std::string cursor = std::get<std::string>(args[1]);

    engine.setDefaultCursor(value, cursor);
}

void fvrFade(Engine& engine, std::vector<ofnx::files::Lst::InstructionParam> args)
{
    if (args.size() != 3) {
        std::cerr << "fvrFade: invalid argument count" << std::endl;
        return;
    }

    if (!std::holds_alternative<double>(args[0]) || !std::holds_alternative<double>(args[1]) || !std::holds_alternative<double>(args[2])) {
        std::cerr << "fvrFade: invalid argument type" << std::endl;
        return;
    }

    double start = std::get<double>(args[0]);
    double end = std::get<double>(args[1]);
    double timer = std::get<double>(args[2]);

    engine.fade(start, end, timer);
}

void fvrEnd(Engine& engine, std::vector<ofnx::files::Lst::InstructionParam> args)
{
    engine.end();
}

void fvrSetAngle(Engine& engine, std::vector<ofnx::files::Lst::InstructionParam> args)
{
    if (args.size() != 2) {
        std::cerr << "fvrSetAngle: invalid argument count" << std::endl;
        return;
    }

    if (!std::holds_alternative<double>(args[0]) || !std::holds_alternative<double>(args[1])) {
        std::cerr << "fvrSetAngle: invalid argument type" << std::endl;
        return;
    }

    int pitchInt = (int)std::get<double>(args[0]) & 0x1fff;
    int yawInt = (int)std::get<double>(args[1]) & 0x1fff;

    if (0xfff < (uint)pitchInt) {
        pitchInt = pitchInt - 0x2000;
    }

    float pitch = pitchInt * 360.0f / 8192.0f; // Convert from 0-8192 range to degree
    float yaw = yawInt * 360.0f / 8192.0f; // Convert from 0-8192 range to degree

    pitch += 90;

    engine.setAngle(pitch, yaw);
}

void fvrHideCursor(Engine& engine, std::vector<ofnx::files::Lst::InstructionParam> args)
{
    if (args.size() != 2) {
        std::cerr << "fvrHideCursor: invalid argument count" << std::endl;
        return;
    }

    if (!std::holds_alternative<std::string>(args[0]) || !std::holds_alternative<double>(args[1])) {
        std::cerr << "fvrHideCursor: invalid argument type" << std::endl;
        return;
    }

    std::string value1 = std::get<std::string>(args[0]);
    double value2 = std::get<double>(args[1]);

    // TODO: implement
    std::cout << "fvrHideCursor: not implemented: " << value1 << " " << value2 << std::endl;
}

void fvrNot(Engine& engine, std::vector<ofnx::files::Lst::InstructionParam> args)
{
    if (args.size() != 1) {
        std::cerr << "fvrNot: invalid argument count" << std::endl;
        return;
    }

    if (!std::holds_alternative<std::string>(args[0])) {
        std::cerr << "fvrNot: invalid argument type" << std::endl;
        return;
    }

    std::string value = std::get<std::string>(args[0]);
    double val = std::get<double>(engine.getStateValue(value));
    engine.setStateValue(value, val == 0.0 ? 1.0 : 0.0);
}

void fvrAngleXMax(Engine& engine, std::vector<ofnx::files::Lst::InstructionParam> args)
{
    if (args.size() != 1) {
        std::cerr << "fvrAngleXMax: invalid argument count" << std::endl;
        return;
    }

    if (!std::holds_alternative<double>(args[0])) {
        std::cerr << "fvrAngleXMax: invalid argument type" << std::endl;
        return;
    }

    double value = std::get<double>(args[0]);

    // TODO: implement
    std::cout << "fvrAngleXMax: not implemented: " << value << std::endl;
}

void fvrAngleYMax(Engine& engine, std::vector<ofnx::files::Lst::InstructionParam> args)
{
    if (args.size() != 1) {
        std::cerr << "fvrAngleYMax: invalid argument count" << std::endl;
        return;
    }

    if (!std::holds_alternative<double>(args[0])) {
        std::cerr << "fvrAngleYMax: invalid argument type" << std::endl;
        return;
    }

    double value = std::get<double>(args[0]);

    // TODO: implement
    std::cout << "fvrAngleXMax: not implemented: " << value << std::endl;
}

void fvrSetZoom(Engine& engine, std::vector<ofnx::files::Lst::InstructionParam> args)
{
    if (args.size() != 1) {
        std::cerr << "fvrSetZoom: invalid argument count" << std::endl;
        return;
    }

    if (!std::holds_alternative<double>(args[0])) {
        std::cerr << "fvrSetZoom: invalid argument type" << std::endl;
        return;
    }

    double value = std::get<double>(args[0]);

    // TODO: implement
    std::cout << "fvrSetZoom: not implemented: " << value << std::endl;
}

void fvrInterpolAngle(Engine& engine, std::vector<ofnx::files::Lst::InstructionParam> args)
{
    if (args.size() != 3) {
        std::cerr << "fvrInterpolAngle: invalid argument count" << std::endl;
        return;
    }

    if (!std::holds_alternative<double>(args[0]) || !std::holds_alternative<double>(args[1]) || !std::holds_alternative<double>(args[2])) {
        std::cerr << "fvrInterpolAngle: invalid argument type" << std::endl;
        return;
    }

    double value1 = std::get<double>(args[0]);
    double value2 = std::get<double>(args[1]);
    double value3 = std::get<double>(args[2]);

    // TODO: implement
    std::cout << "fvrInterpolAngle: not implemented: " << value1 << " " << value2 << " " << value3 << std::endl;
}

void registerScriptFunctions(Engine& engine)
{
    // New registration system
    engine.registerScriptFunction("gotowarp", &fvrGotoWarp);
    engine.registerScriptFunction("playsound", &fvrPlaySound);
    engine.registerScriptFunction("stopsound", &fvrStopSound);
    engine.registerScriptFunction("playsound3d", &fvrPlaySound3d);
    engine.registerScriptFunction("stopsound3d", &fvrStopSound3d);
    engine.registerScriptFunction("playmusique", &fvrPlayMusic);
    engine.registerScriptFunction("stopmusique", &fvrStopMusic);
    engine.registerScriptFunction("set", &fvrSet);
    engine.registerScriptFunction("lockkey", &fvrLockKey);
    engine.registerScriptFunction("resetlockkey", &fvrResetLockKey);
    engine.registerScriptFunction("setcursor", &fvrSetCursor);
    engine.registerScriptFunction("setcursordefault", &fvrSetCursorDefault);
    engine.registerScriptFunction("fade", &fvrFade);
    engine.registerScriptFunction("end", &fvrEnd);
    engine.registerScriptFunction("setangle", &fvrSetAngle);
    engine.registerScriptFunction("hidecursor", &fvrHideCursor);
    engine.registerScriptFunction("not", &fvrNot);
    engine.registerScriptFunction("anglexmax", &fvrAngleXMax);
    engine.registerScriptFunction("angleymax", &fvrAngleYMax);
    engine.registerScriptFunction("setzoom", &fvrSetZoom);
    engine.registerScriptFunction("interpolangle", &fvrInterpolAngle);
}

#include "pluginlouvre.h"

#include <iostream>
#include <string>
#include <thread>

struct LouvreData {
    std::vector<uint32_t> objectInventory;
};

LouvreData g_louvreData;

void plgPlayMovie(Engine& engine, std::vector<FvrScript::InstructionParam> args)
{
    if (args.size() != 1) {
        std::cerr << "plgPlayMovie: invalid number of arguments" << std::endl;
        return;
    }

    if (!std::holds_alternative<std::string>(args[0])) {
        std::cerr << "plgPlayMovie: argument is not a string" << std::endl;
        return;
    }

    std::string movie = std::get<std::string>(args[0]);

    engine.playMovie(movie);
}

void plgCmp(Engine& engine, std::vector<FvrScript::InstructionParam> args)
{
    if (args.size() != 5) {
        std::cerr << "plgCmp: invalid number of arguments" << std::endl;
        return;
    }

    if (!std::holds_alternative<std::string>(args[0]) || !std::holds_alternative<std::string>(args[1]) || !std::holds_alternative<std::string>(args[2]) || !std::holds_alternative<std::string>(args[3]) || !std::holds_alternative<double>(args[4])) {
        std::cerr << "plgCmp: invalid arguments" << std::endl;
        return;
    }

    std::string cond = std::get<std::string>(args[0]);
    std::string notCond = std::get<std::string>(args[1]);
    std::string var = std::get<std::string>(args[2]);
    std::string op = std::get<std::string>(args[3]);
    double value = std::get<double>(args[4]);

    bool result = false;
    if (op == "==") {
        result = std::get<double>(engine.getStateValue(var)) == value;
    } else if (op == "!=") {
        result = std::get<double>(engine.getStateValue(var)) != value;
    } else if (op == "<") {
        result = std::get<double>(engine.getStateValue(var)) < value;
    } else if (op == "<=") {
        result = std::get<double>(engine.getStateValue(var)) <= value;
    } else if (op == ">") {
        result = std::get<double>(engine.getStateValue(var)) > value;
    } else if (op == ">=") {
        result = std::get<double>(engine.getStateValue(var)) >= value;
    } else {
        std::cerr << "plgCmp: invalid operator: " << op << std::endl;
    }

    engine.setStateValue(cond, result ? 1.0 : 0.0);
    engine.setStateValue(notCond, result ? 0.0 : 1.0);
}

void plgKillTimer(Engine& engine, std::vector<FvrScript::InstructionParam> args)
{
    // TODO: implement
    std::cout << "plgKillTimer: not implemented" << std::endl;
}

void plgCarteDestination(Engine& engine, std::vector<FvrScript::InstructionParam> args)
{
    if (args.size() != 2) {
        std::cerr << "plgCarteDestination: invalid number of arguments" << std::endl;
        return;
    }

    if (!std::holds_alternative<std::string>(args[0]) || !std::holds_alternative<std::string>(args[1])) {
        std::cerr << "plgCarteDestination: invalid arguments" << std::endl;
        return;
    }

    std::string x = std::get<std::string>(args[0]);
    std::string y = std::get<std::string>(args[1]);

    // TODO: implement
    std::cout << "plgCarteDestination: not implemented: " << x << " " << y << std::endl;
}

void plgPauseTimer(Engine& engine, std::vector<FvrScript::InstructionParam> args)
{
    if (args.size() != 2) {
        std::cerr << "plgPauseTimer: invalid number of arguments" << std::endl;
        return;
    }

    if (!std::holds_alternative<double>(args[0]) || !std::holds_alternative<double>(args[1])) {
        std::cerr << "plgPauseTimer: invalid arguments" << std::endl;
        return;
    }

    double timer = std::get<double>(args[0]);
    double value = std::get<double>(args[1]);

    // TODO: implement
    std::cout << "plgPauseTimer: not implemented: " << timer << " " << value << std::endl;
}

void plgAdd(Engine& engine, std::vector<FvrScript::InstructionParam> args)
{
    if (args.size() != 3) {
        std::cerr << "plgAdd: invalid number of arguments" << std::endl;
        return;
    }

    if (!std::holds_alternative<std::string>(args[0]) || !std::holds_alternative<std::string>(args[1]) || !std::holds_alternative<std::string>(args[2])) {
        std::cerr << "plgAdd: invalid arguments" << std::endl;
        return;
    }

    std::string object = std::get<std::string>(args[0]);
    std::string cond = std::get<std::string>(args[1]);
    std::string notCond = std::get<std::string>(args[2]);

    // TODO: implement
    std::cout << "plgAdd: not implemented: " << object << " " << cond << " " << notCond << std::endl;
}

void plgInitCoffre(Engine& engine, std::vector<FvrScript::InstructionParam> args)
{
    // TODO: implement
    std::cout << "plgInitCoffre: not implemented" << std::endl;
}

void plgAffichePortef(Engine& engine, std::vector<FvrScript::InstructionParam> args)
{
    if (args.size() != 1) {
        std::cerr << "plgAffichePortef: invalid number of arguments" << std::endl;
        return;
    }

    if (!std::holds_alternative<double>(args[0])) {
        std::cerr << "plgAffichePortef: invalid arguments" << std::endl;
        return;
    }

    double value = std::get<double>(args[0]);

    // TODO: implement
    std::cout << "plgAffichePortef: not implemented: " << value << std::endl;
}

void plgAfficheCoffre(Engine& engine, std::vector<FvrScript::InstructionParam> args)
{
    // TODO: implement
    std::cout << "plgAfficheCoffre: not implemented" << std::endl;
}

void plgScroll(Engine& engine, std::vector<FvrScript::InstructionParam> args)
{
    if (args.size() != 1) {
        std::cerr << "plgScroll: invalid number of arguments" << std::endl;
        return;
    }

    if (!std::holds_alternative<double>(args[0])) {
        std::cerr << "plgScroll: invalid arguments" << std::endl;
        return;
    }

    double value = std::get<double>(args[0]);

    // TODO: implement
    std::cout << "plgScroll: not implemented: " << value << std::endl;
}

void plgSelectPorteF(Engine& engine, std::vector<FvrScript::InstructionParam> args)
{
    if (args.size() != 2) {
        std::cerr << "plgSelectPorteF: invalid number of arguments" << std::endl;
        return;
    }

    if (!std::holds_alternative<double>(args[0]) || !std::holds_alternative<std::string>(args[1])) {
        std::cerr << "plgSelectPorteF: invalid arguments" << std::endl;
        return;
    }

    double value = std::get<double>(args[0]);
    std::string z = std::get<std::string>(args[1]);

    // TODO: implement
    std::cout << "plgSelectPorteF: not implemented: " << value << " " << z << std::endl;
}

void plgPlayAnimBloc(Engine& engine, std::vector<FvrScript::InstructionParam> args)
{
    if (args.size() != 4) {
        std::cerr << "plgPlayAnimBloc: invalid number of arguments" << std::endl;
        return;
    }

    if (!std::holds_alternative<std::string>(args[0]) || !std::holds_alternative<std::string>(args[1]) || !std::holds_alternative<double>(args[2]) || !std::holds_alternative<double>(args[3])) {
        std::cerr << "plgPlayAnimBloc: invalid arguments" << std::endl;
        return;
    }

    std::string name = std::get<std::string>(args[0]);
    std::string var = std::get<std::string>(args[1]);
    double tmp1 = std::get<double>(args[2]);
    double tmp2 = std::get<double>(args[3]);

    // TODO: implement correctly
    engine.playAnim(name);
}

void plgUntilLoop(Engine& engine, std::vector<FvrScript::InstructionParam> args)
{
    if (args.size() != 2) {
        std::cerr << "plgUntilLoop: invalid number of arguments" << std::endl;
        return;
    }

    if (!std::holds_alternative<std::string>(args[0]) || !std::holds_alternative<double>(args[1])) {
        std::cerr << "plgUntilLoop: invalid arguments" << std::endl;
        return;
    }

    std::string y = std::get<std::string>(args[0]);
    double value = std::get<double>(args[1]);

    // TODO: implement
    std::cout << "plgUntilLoop: not implemented: " << y << " " << value << std::endl;
}

void plgMemoryRelease(Engine& engine, std::vector<FvrScript::InstructionParam> args)
{
    // TODO: implement
    std::cout << "plgMemoryRelease: not implemented" << std::endl;
}

void plgSelectCoffre(Engine& engine, std::vector<FvrScript::InstructionParam> args)
{
    if (args.size() != 2) {
        std::cerr << "plgSelectCoffre: invalid number of arguments" << std::endl;
        return;
    }

    if (!std::holds_alternative<double>(args[0]) || !std::holds_alternative<std::string>(args[1])) {
        std::cerr << "plgSelectCoffre: invalid arguments" << std::endl;
        return;
    }

    double value = std::get<double>(args[0]);
    std::string z = std::get<std::string>(args[1]);

    // TODO: implement
    std::cout << "plgSelectCoffre: not implemented: " << value << " " << z << std::endl;
}

void plgPlayAnimBlocNumber(Engine& engine, std::vector<FvrScript::InstructionParam> args)
{
    if (args.size() != 5) {
        std::cerr << "plgPlayAnimBlocNumber: invalid number of arguments" << std::endl;
        return;
    }

    if (!std::holds_alternative<std::string>(args[0]) || !std::holds_alternative<std::string>(args[1]) || !std::holds_alternative<std::string>(args[2]) || !std::holds_alternative<double>(args[3]) || !std::holds_alternative<double>(args[4])) {
        std::cerr << "plgPlayAnimBlocNumber: invalid arguments" << std::endl;
        return;
    }

    std::string value = std::get<std::string>(args[0]);
    std::string y = std::get<std::string>(args[1]);
    std::string z = std::get<std::string>(args[2]);
    double value1 = std::get<double>(args[3]);
    double value2 = std::get<double>(args[4]);

    // TODO: implement
    std::cout << "plgPlayAnimBlocNumber: not implemented: " << value << " " << y << " " << z << " " << value1 << " " << value2 << std::endl;
}

void plgSub(Engine& engine, std::vector<FvrScript::InstructionParam> args)
{
    if (args.size() != 3) {
        std::cerr << "plgSub: invalid number of arguments" << std::endl;
        return;
    }

    if (!std::holds_alternative<std::string>(args[0]) || !std::holds_alternative<std::string>(args[1]) || !std::holds_alternative<double>(args[2])) {
        std::cerr << "plgSub: invalid arguments" << std::endl;
        return;
    }

    std::string value = std::get<std::string>(args[0]);
    std::string y = std::get<std::string>(args[1]);
    double value2 = std::get<double>(args[2]);

    // TODO: implement
    std::cout << "plgSub: not implemented: " << value << " " << y << " " << value << std::endl;
}

void plgWhileLoop(Engine& engine, std::vector<FvrScript::InstructionParam> args)
{
    if (args.size() != 1) {
        std::cerr << "plgWhileLoop: invalid number of arguments" << std::endl;
        return;
    }

    if (!std::holds_alternative<double>(args[0])) {
        std::cerr << "plgWhileLoop: invalid arguments" << std::endl;
        return;
    }

    double value = std::get<double>(args[0]);

    // TODO: implement
    std::cout << "plgWhileLoop: not implemented: " << value << std::endl;
}

void plgIsHere(Engine& engine, std::vector<FvrScript::InstructionParam> args)
{
    // TODO: implement
    std::cout << "plgIsHere: not implemented" << std::endl;
}

void plgDrawTextSelection(Engine& engine, std::vector<FvrScript::InstructionParam> args)
{
    // TODO: implement
    std::cout << "plgDrawTextSelection: not implemented" << std::endl;
}

void plgPorteFRollover(Engine& engine, std::vector<FvrScript::InstructionParam> args)
{
    if (args.size() != 1) {
        std::cerr << "plgPorteFRollover: invalid number of arguments" << std::endl;
        return;
    }

    if (!std::holds_alternative<double>(args[0])) {
        std::cerr << "plgPorteFRollover: invalid arguments" << std::endl;
        return;
    }

    double value = std::get<double>(args[0]);

    // TODO: implement
    std::cout << "plgPorteFRollover: not implemented: " << value << std::endl;
}

void plgSetMonde4(Engine& engine, std::vector<FvrScript::InstructionParam> args)
{
    if (args.size() != 1) {
        std::cerr << "plgSetMonde4: invalid number of arguments" << std::endl;
        return;
    }

    if (!std::holds_alternative<double>(args[0])) {
        std::cerr << "plgSetMonde4: invalid arguments" << std::endl;
        return;
    }

    double value = std::get<double>(args[0]);

    // TODO: implement
    std::cout << "plgSetMonde4: not implemented: " << value << std::endl;
}

void plgChangeCurseur(Engine& engine, std::vector<FvrScript::InstructionParam> args)
{
    if (args.size() != 1) {
        std::cerr << "plgChangeCurseur: invalid number of arguments" << std::endl;
        return;
    }

    if (!std::holds_alternative<double>(args[0])) {
        std::cerr << "plgChangeCurseur: invalid arguments" << std::endl;
        return;
    }

    double value = std::get<double>(args[0]);

    // TODO: implement
    std::cout << "plgChangeCurseur: not implemented: " << value << std::endl;
}

void plgLoadSaveContextRestored(Engine& engine, std::vector<FvrScript::InstructionParam> args)
{
    if (args.size() != 2) {
        std::cerr << "plgLoadSaveContextRestored: invalid number of arguments" << std::endl;
        return;
    }

    if (!std::holds_alternative<std::string>(args[0]) || !std::holds_alternative<std::string>(args[1])) {
        std::cerr << "plgLoadSaveContextRestored: invalid arguments" << std::endl;
        return;
    }

    std::string reloading = std::get<std::string>(args[0]);
    std::string reloadDone = std::get<std::string>(args[1]);

    // TODO: implement
    std::cout << "plgLoadSaveContextRestored: not implemented: " << reloading << " " << reloadDone << std::endl;
}

void plgLoadSaveCaptureContext(Engine& engine, std::vector<FvrScript::InstructionParam> args)
{
    // TODO: implement
    std::cout << "plgLoadSaveCaptureContext: not implemented" << std::endl;
}

void plgLoadSaveInitSlots(Engine& engine, std::vector<FvrScript::InstructionParam> args)
{
    if (args.size() != 1) {
        std::cerr << "plgLoadSaveInitSlots: invalid number of arguments" << std::endl;
        return;
    }

    if (!std::holds_alternative<double>(args[0])) {
        std::cerr << "plgLoadSaveInitSlots: invalid arguments" << std::endl;
        return;
    }

    double value = std::get<double>(args[0]);

    // TODO: implement
    std::cout << "plgLoadSaveInitSlots: not implemented: " << value << std::endl;
}

void plgLoadSaveSave(Engine& engine, std::vector<FvrScript::InstructionParam> args)
{
    if (args.size() != 1) {
        std::cerr << "plgLoadSaveSave: invalid number of arguments" << std::endl;
        return;
    }

    if (!std::holds_alternative<double>(args[0])) {
        std::cerr << "plgLoadSaveSave: invalid arguments" << std::endl;
        return;
    }

    double value = std::get<double>(args[0]);

    // TODO: implement
    std::cout << "plgLoadSaveSave: not implemented: " << value << std::endl;
}

void plgMultiCdSetNextScript(Engine& engine, std::vector<FvrScript::InstructionParam> args)
{
    if (args.size() != 1) {
        std::cerr << "plgMultiCdSetNextScript: invalid number of arguments" << std::endl;
        return;
    }

    if (!std::holds_alternative<std::string>(args[0])) {
        std::cerr << "plgMultiCdSetNextScript: invalid arguments" << std::endl;
        return;
    }

    std::string value = std::get<std::string>(args[0]);

    // TODO: implement
    std::cout << "plgMultiCdSetNextScript: not implemented: " << value << std::endl;
}

void plgReset(Engine& engine, std::vector<FvrScript::InstructionParam> args)
{
    // TODO: implement
    std::cout << "plgReset: not implemented" << std::endl;
}

void plgAddObject(Engine& engine, std::vector<FvrScript::InstructionParam> args)
{
    if (args.size() != 3) {
        std::cerr << "plgAddObject: invalid number of arguments" << std::endl;
        return;
    }

    if (!std::holds_alternative<double>(args[0]) || !std::holds_alternative<std::string>(args[1]) || !std::holds_alternative<std::string>(args[2])) {
        std::cerr << "plgAddObject: invalid arguments" << std::endl;
        return;
    }

    double value = std::get<double>(args[0]);
    std::string cond = std::get<std::string>(args[1]);
    std::string notCond = std::get<std::string>(args[2]);

    // TODO: implement correctly
    g_louvreData.objectInventory.push_back((int)value);
    engine.setStateValue(cond, 1.0);
    engine.setStateValue(notCond, 0.0);
}

void plgAddCoffreObject(Engine& engine, std::vector<FvrScript::InstructionParam> args)
{
    if (args.size() != 1) {
        std::cerr << "plgAddCoffreObject: invalid number of arguments" << std::endl;
        return;
    }

    if (!std::holds_alternative<double>(args[0])) {
        std::cerr << "plgAddCoffreObject: invalid arguments" << std::endl;
        return;
    }

    double value = std::get<double>(args[0]);

    // TODO: implement
    std::cout << "plgAddCoffreObject: not implemented: " << value << std::endl;
}

void plgIsPresent(Engine& engine, std::vector<FvrScript::InstructionParam> args)
{
    if (args.size() != 3) {
        std::cerr << "plgIsPresent: invalid number of arguments" << std::endl;
        return;
    }

    if (!std::holds_alternative<double>(args[0]) || !std::holds_alternative<std::string>(args[1]) || !std::holds_alternative<std::string>(args[2])) {
        std::cerr << "plgIsPresent: invalid arguments" << std::endl;
        return;
    }

    double value = std::get<double>(args[0]);
    std::string cond = std::get<std::string>(args[1]);
    std::string notCond = std::get<std::string>(args[2]);

    // TODO: implement correctly
    bool result = std::find(g_louvreData.objectInventory.begin(), g_louvreData.objectInventory.end(), value) != g_louvreData.objectInventory.end();
    engine.setStateValue(cond, result ? 1.0 : 0.0);
    engine.setStateValue(notCond, result ? 0.0 : 1.0);
}

void plgRemoveObject(Engine& engine, std::vector<FvrScript::InstructionParam> args)
{
    if (args.size() != 1) {
        std::cerr << "plgRemoveObject: invalid number of arguments" << std::endl;
        return;
    }

    if (!std::holds_alternative<double>(args[0])) {
        std::cerr << "plgRemoveObject: invalid arguments" << std::endl;
        return;
    }

    double value = std::get<double>(args[0]);

    // TODO: implement correctly
    g_louvreData.objectInventory.erase(std::remove(g_louvreData.objectInventory.begin(), g_louvreData.objectInventory.end(), value), g_louvreData.objectInventory.end());
}

void plgStartTimer(Engine& engine, std::vector<FvrScript::InstructionParam> args)
{
    if (args.size() != 1) {
        std::cerr << "plgStartTimer: invalid number of arguments" << std::endl;
        return;
    }

    if (!std::holds_alternative<double>(args[0])) {
        std::cerr << "plgStartTimer: invalid arguments" << std::endl;
        return;
    }

    double value = std::get<double>(args[0]);

    // TODO: implement
    std::cout << "plgStartTimer: not implemented: " << value << std::endl;
}

void plgLoadSaveTestSlot(Engine& engine, std::vector<FvrScript::InstructionParam> args)
{
    if (args.size() != 3) {
        std::cerr << "plgLoadSaveTestSlot: invalid number of arguments" << std::endl;
        return;
    }

    if (!std::holds_alternative<double>(args[0]) || !std::holds_alternative<std::string>(args[1]) || !std::holds_alternative<std::string>(args[2])) {
        std::cerr << "plgLoadSaveTestSlot: invalid arguments" << std::endl;
        return;
    }

    double value = std::get<double>(args[0]);
    std::string cond = std::get<std::string>(args[1]);
    std::string notCond = std::get<std::string>(args[2]);

    // TODO: implement
    std::cout << "plgLoadSaveTestSlot: not implemented: " << value << " " << cond << " " << notCond << std::endl;
}

void plgLoadSaveLoad(Engine& engine, std::vector<FvrScript::InstructionParam> args)
{
    if (args.size() != 1) {
        std::cerr << "plgLoadSaveLoad: invalid number of arguments" << std::endl;
        return;
    }

    if (!std::holds_alternative<double>(args[0])) {
        std::cerr << "plgLoadSaveLoad: invalid arguments" << std::endl;
        return;
    }

    double value = std::get<double>(args[0]);

    // TODO: implement
    std::cout << "plgLoadSaveLoad: not implemented: " << value << std::endl;
}

void plgSaveCoffre(Engine& engine, std::vector<FvrScript::InstructionParam> args)
{
    // TODO: implement
    std::cout << "plgSaveCoffre: not implemented" << std::endl;
}

void plgLoadSaveSetContextLabel(Engine& engine, std::vector<FvrScript::InstructionParam> args)
{
    if (args.size() != 1) {
        std::cerr << "plgLoadSaveSetContextLabel: invalid number of arguments" << std::endl;
        return;
    }

    if (!std::holds_alternative<std::string>(args[0])) {
        std::cerr << "plgLoadSaveSetContextLabel: invalid arguments" << std::endl;
        return;
    }

    std::string value = std::get<std::string>(args[0]);

    // TODO: implement
    std::cout << "plgLoadSaveSetContextLabel: not implemented: " << value << std::endl;
}

void plgLoadSaveDrawSlot(Engine& engine, std::vector<FvrScript::InstructionParam> args)
{
    if (args.size() != 4) {
        std::cerr << "plgLoadSaveDrawSlot: invalid number of arguments" << std::endl;
        return;
    }

    if (!std::holds_alternative<double>(args[0]) || !std::holds_alternative<double>(args[1]) || !std::holds_alternative<double>(args[2]) || !std::holds_alternative<double>(args[3])) {
        std::cerr << "plgLoadSaveDrawSlot: invalid arguments" << std::endl;
        return;
    }

    double value1 = std::get<double>(args[0]);
    double value2 = std::get<double>(args[1]);
    double value3 = std::get<double>(args[2]);
    double value4 = std::get<double>(args[3]);

    // TODO: implement
    std::cout << "plgLoadSaveDrawSlot: not implemented: " << value1 << " " << value2 << " " << value3 << " " << value4 << std::endl;
}

void plgEnd(Engine& engine, std::vector<FvrScript::InstructionParam> args)
{
    engine.end();
}

void plgInit2(Engine& engine, std::vector<FvrScript::InstructionParam> args)
{
    if (args.size() != 1) {
        std::cerr << "plgInit2: invalid number of arguments" << std::endl;
        return;
    }

    if (!std::holds_alternative<std::string>(args[0])) {
        std::cerr << "plgInit2: invalid arguments" << std::endl;
        return;
    }

    std::string variable = std::get<std::string>(args[0]);

    // TODO: implement
    std::cout << "plgInit2: not implemented: " << variable << std::endl;
    engine.setStateValue(variable, 0.0);
}

void plgInit(Engine& engine, std::vector<FvrScript::InstructionParam> args)
{
    if (args.size() != 2) {
        std::cerr << "plgInit: invalid number of arguments" << std::endl;
        return;
    }

    if (!std::holds_alternative<double>(args[0]) || !std::holds_alternative<std::string>(args[1])) {
        std::cerr << "plgInit: invalid arguments" << std::endl;
        return;
    }

    double value = std::get<double>(args[0]);
    std::string variable = std::get<std::string>(args[1]);

    // TODO: implement
    std::cout << "plgInit: not implemented: " << value << " " << variable << std::endl;
    engine.setStateValue(variable, 0.0);
}

void plgLoadSaveEnterScript(Engine& engine, std::vector<FvrScript::InstructionParam> args)
{
    if (args.size() != 2) {
        std::cerr << "plgLoadSaveEnterScript: invalid number of arguments" << std::endl;
        return;
    }

    if (!std::holds_alternative<std::string>(args[0]) || !std::holds_alternative<std::string>(args[1])) {
        std::cerr << "plgLoadSaveEnterScript: invalid arguments" << std::endl;
        return;
    }

    std::string var = std::get<std::string>(args[0]);
    std::string notVar = std::get<std::string>(args[1]);

    // TODO: implement
    std::cout << "plgLoadSaveEnterScript: not implemented: " << var << " " << notVar << std::endl;
    engine.setStateValue(var, 0.0); // Reloading
    engine.setStateValue(notVar, 1.0); // Not reloading
}

void plgLoadCoffre(Engine& engine, std::vector<FvrScript::InstructionParam> args)
{
    // TODO: implement
    std::cout << "plgLoadCoffre: not implemented" << std::endl;
}

void plgGetMonde4(Engine& engine, std::vector<FvrScript::InstructionParam> args)
{
    if (args.size() != 2) {
        std::cerr << "plgGetMonde4: invalid number of arguments" << std::endl;
        return;
    }

    if (!std::holds_alternative<std::string>(args[0]) || !std::holds_alternative<std::string>(args[1])) {
        std::cerr << "plgGetMonde4: invalid arguments" << std::endl;
        return;
    }

    std::string cond = std::get<std::string>(args[0]);
    std::string notCond = std::get<std::string>(args[1]);

    // TODO: implement
    std::cout << "plgGetMonde4: not implemented: " << cond << " " << notCond << std::endl;
    engine.setStateValue(cond, 0.0);
    engine.setStateValue(notCond, 1.0);
}

void plgAfficheSelection(Engine& engine, std::vector<FvrScript::InstructionParam> args)
{
    // TODO: implement
    std::cout << "plgAfficheSelection: not implemented" << std::endl;
}

void plgSelect(Engine& engine, std::vector<FvrScript::InstructionParam> args)
{
    if (args.size() != 3) {
        std::cerr << "plgSelect: invalid number of arguments" << std::endl;
        return;
    }

    if (!std::holds_alternative<double>(args[0]) || !std::holds_alternative<std::string>(args[1]) || !std::holds_alternative<std::string>(args[2])) {
        std::cerr << "plgSelect: invalid arguments" << std::endl;
        return;
    }

    double value = std::get<double>(args[0]);
    std::string w = std::get<std::string>(args[1]);
    std::string z = std::get<std::string>(args[2]);

    // TODO: implement
    std::cout << "plgSelect: not implemented: " << value << " " << w << " " << z << std::endl;
}

void plgDoAction(Engine& engine, std::vector<FvrScript::InstructionParam> args)
{
    if (args.size() != 2) {
        std::cerr << "plgDoAction: invalid number of arguments" << std::endl;
        return;
    }

    if (!std::holds_alternative<double>(args[0]) || !std::holds_alternative<std::string>(args[1])) {
        std::cerr << "plgDoAction: invalid arguments" << std::endl;
        return;
    }

    double value = std::get<double>(args[0]);
    std::string z = std::get<std::string>(args[1]);

    // TODO: implement
    std::cout << "plgDoAction: not implemented: " << value << " " << z << std::endl;
}

void plgDiscocier(Engine& engine, std::vector<FvrScript::InstructionParam> args)
{
    if (args.size() != 1) {
        std::cerr << "plgDiscocier: invalid number of arguments" << std::endl;
        return;
    }

    std::string z = std::get<std::string>(args[0]);

    // TODO: implement
    std::cout << "plgDiscocier: not implemented: " << z << std::endl;
}

void registerPluginLouvre(Engine& engine)
{
    std::cout << "Registering plugin" << std::endl;

    engine.registerScriptFunction("play_movie", &plgPlayMovie);
    engine.registerScriptFunction("cmp", &plgCmp);
    engine.registerScriptFunction("killtimer", &plgKillTimer);
    engine.registerScriptFunction("cartedestination", &plgCarteDestination);
    engine.registerScriptFunction("pausetimer", &plgPauseTimer);
    engine.registerScriptFunction("add", &plgAdd);
    engine.registerScriptFunction("initcoffre", &plgInitCoffre);
    engine.registerScriptFunction("afficheportef", &plgAffichePortef);
    engine.registerScriptFunction("affichecoffre", &plgAfficheCoffre);
    engine.registerScriptFunction("scroll", &plgScroll);
    engine.registerScriptFunction("selectportef", &plgSelectPorteF);
    engine.registerScriptFunction("play_animbloc", &plgPlayAnimBloc);
    engine.registerScriptFunction("until", &plgUntilLoop);
    engine.registerScriptFunction("memoryrelease", &plgMemoryRelease);
    engine.registerScriptFunction("selectcoffre", &plgSelectCoffre);
    engine.registerScriptFunction("play_animbloc_number", &plgPlayAnimBlocNumber);
    engine.registerScriptFunction("sub", &plgSub);
    engine.registerScriptFunction("while", &plgWhileLoop);
    engine.registerScriptFunction("ishere", &plgIsHere);
    engine.registerScriptFunction("drawtextselection", &plgDrawTextSelection);
    engine.registerScriptFunction("portefrollover", &plgPorteFRollover);
    engine.registerScriptFunction("setmonde4", &plgSetMonde4);
    engine.registerScriptFunction("changecurseur", &plgChangeCurseur);
    engine.registerScriptFunction("loadsave_context_restored", &plgLoadSaveContextRestored);
    engine.registerScriptFunction("loadsave_capture_context", &plgLoadSaveCaptureContext);
    engine.registerScriptFunction("loadsave_init_slots", &plgLoadSaveInitSlots);
    engine.registerScriptFunction("loadsave_save", &plgLoadSaveSave);
    engine.registerScriptFunction("multicd_set_next_script", &plgMultiCdSetNextScript);
    engine.registerScriptFunction("reset", &plgReset);
    engine.registerScriptFunction("addobject", &plgAddObject);
    engine.registerScriptFunction("addcoffreobject", &plgAddCoffreObject);
    engine.registerScriptFunction("ispresent", &plgIsPresent);
    engine.registerScriptFunction("removeobject", &plgRemoveObject);
    engine.registerScriptFunction("starttimer", &plgStartTimer);
    engine.registerScriptFunction("loadsave_test_slot", &plgLoadSaveTestSlot);
    engine.registerScriptFunction("loadsave_load", &plgLoadSaveLoad);
    engine.registerScriptFunction("savecoffre", &plgSaveCoffre);
    engine.registerScriptFunction("loadsave_set_context_label", &plgLoadSaveSetContextLabel);
    engine.registerScriptFunction("loadsave_draw_slot", &plgLoadSaveDrawSlot);
    engine.registerScriptFunction("end", &plgEnd);
    engine.registerScriptFunction("init2", &plgInit2);
    engine.registerScriptFunction("init", &plgInit);
    engine.registerScriptFunction("loadsave_enter_script", &plgLoadSaveEnterScript);
    engine.registerScriptFunction("loadcoffre", &plgLoadCoffre);
    engine.registerScriptFunction("getmonde4", &plgGetMonde4);
    engine.registerScriptFunction("afficheselection", &plgAfficheSelection);
    engine.registerScriptFunction("select", &plgSelect);
    engine.registerScriptFunction("doaction", &plgDoAction);
    engine.registerScriptFunction("discocier", &plgDiscocier);
}

#ifndef LSTSCRIPT_H
#define LSTSCRIPT_H

#include "liblstparser_globals.h"

#include <string>
#include <variant>
#include <vector>

class LIBLSTPARSER_EXPORT LstScript {
public:
    using InstructionParam = std::variant<int, double, std::string>;
    struct Instruction {
        std::string name;
        std::vector<InstructionParam> params;
        std::vector<Instruction> subInstructions; // For ifand/ifor/plugin/subroutine
    };
    using InstructionBlock = std::vector<Instruction>;

public:
    LstScript();
    ~LstScript();

    bool addVariable(const std::string& name);
    bool addInstruction(const std::string& warpName, const int& testId, const Instruction& instruction);
    bool addSubroutine(const Instruction& instruction);

    void optimize();

    InstructionBlock& getInitBlock(const std::string& warpName);
    InstructionBlock& getTestBlock(const std::string& warpName, const int& testId);

    bool saveText(const std::string& fileOut);

private:
    class LstScriptPrivate;
    LstScriptPrivate* d_ptr;
};

#endif // LSTSCRIPT_H

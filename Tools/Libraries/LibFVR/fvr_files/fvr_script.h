#ifndef FVR_SCRIPT_H
#define FVR_SCRIPT_H

#include "libfvr_globals.h"

#include <string>
#include <variant>
#include <vector>

class LIBFVR_EXPORT FvrScript final {
public:
    using InstructionParam = std::variant<int, double, std::string>;
    struct Instruction {
        std::string name;
        std::vector<InstructionParam> params;
        std::vector<Instruction> subInstructions; // For ifand/ifor/plugin/subroutine
    };
    using InstructionBlock = std::vector<Instruction>;

public:
    FvrScript();
    ~FvrScript();

    bool parseLst(const std::string& fileName);

    InstructionBlock& getInitBlock(const std::string& warpName);
    InstructionBlock& getTestBlock(const std::string& warpName, const int& testId);

private:
    class FvrScriptPrivate;
    FvrScriptPrivate* d_ptr;
};

#endif // FVR_SCRIPT_H

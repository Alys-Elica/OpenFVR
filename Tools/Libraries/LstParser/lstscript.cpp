#include "lstscript.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <set>

/* Private */
class LstScript::LstScriptPrivate {
    friend class LstScript;

public:
    struct Warp {
        std::string name;
        InstructionBlock initBlock;
        std::map<uint32_t, InstructionBlock> testBlockList;
    };

private:
    std::set<std::string> m_listVariables;
    std::map<std::string, Warp> m_listWarps;
    std::map<std::string, Instruction> m_listSubroutines;

    std::string initWarp;
};

/* Public */
LstScript::LstScript() { d_ptr = new LstScriptPrivate; }

LstScript::~LstScript() { delete d_ptr; }

bool LstScript::addVariable(const std::string& name)
{
    d_ptr->m_listVariables.insert(name);

    return true;
}

bool LstScript::addInstruction(
    const std::string& warpName, const int& testId,
    const Instruction& instruction)
{
    // Register warp
    if (d_ptr->m_listWarps.empty()) {
        d_ptr->initWarp = warpName;
    }

    if (d_ptr->m_listWarps.find(warpName) == d_ptr->m_listWarps.end()) {
        LstScriptPrivate::Warp warp;
        warp.name = warpName;
        d_ptr->m_listWarps[warpName] = warp;
    }

    // Add instruction
    if (testId == -1) {
        d_ptr->m_listWarps[warpName].initBlock.push_back(instruction);
        return true;
    }

    // Register test
    if (d_ptr->m_listWarps[warpName].testBlockList.empty()) {
        d_ptr->m_listWarps[warpName].testBlockList[testId] = InstructionBlock();
    }

    if (testId < 0) {
        return false;
    }

    d_ptr->m_listWarps[warpName].testBlockList[testId].push_back(instruction);

    return true;
}

bool LstScript::addSubroutine(const Instruction& instruction)
{
    d_ptr->m_listSubroutines[instruction.name] = instruction;

    return true;
}

void optimizeInstructionBlock(LstScript::InstructionBlock& block)
{
    for (auto it = block.begin(); it != block.end();) {
        // Combine identical ifand/ifor blocks
        if (it->name == "ifand" || it->name == "ifor") {
            if (it != block.end() - 1) {
                if ((it + 1)->name == it->name && (it + 1)->params == it->params) {
                    it->subInstructions.insert(
                        it->subInstructions.end(),
                        (it + 1)->subInstructions.begin(),
                        (it + 1)->subInstructions.end());
                    block.erase(it + 1);
                    continue;
                }
            }
        }
        ++it;
    }
}

void LstScript::optimize()
{
    for (auto& warp : d_ptr->m_listWarps) {
        for (auto& test : warp.second.testBlockList) {
            optimizeInstructionBlock(test.second);
        }
    }

    for (auto& subroutine : d_ptr->m_listSubroutines) {
        optimizeInstructionBlock(subroutine.second.subInstructions);
    }

    for (auto& warp : d_ptr->m_listWarps) {
        optimizeInstructionBlock(warp.second.initBlock);
    }
}

LstScript::InstructionBlock& LstScript::getInitBlock(const std::string& warpName)
{
    return d_ptr->m_listWarps[warpName].initBlock;
}

LstScript::InstructionBlock& LstScript::getTestBlock(
    const std::string& warpName, const int& testId)
{
    return d_ptr->m_listWarps[warpName].testBlockList[testId];
}

void instructionToText(std::ofstream& output,
    const LstScript::Instruction& instruction, int level)
{
    for (int i = 0; i < level; i++) {
        output << "\t";
    }

    output << instruction.name;

    if (!instruction.params.empty()) {
        output << "(";
        for (size_t i = 0; i < instruction.params.size(); i++) {
            if (std::holds_alternative<int>(instruction.params[i])) {
                output << std::get<int>(instruction.params[i]);
            } else if (std::holds_alternative<double>(instruction.params[i])) {
                output << std::get<double>(instruction.params[i]);
            } else if (std::holds_alternative<std::string>(instruction.params[i])) {
                output << "\"" << std::get<std::string>(instruction.params[i])
                       << "\"";
            }

            if (i < instruction.params.size() - 1) {
                output << ",";
            }
        }
        output << ")";
    }

    output << std::endl;

    for (const LstScript::Instruction& subInstruction :
        instruction.subInstructions) {
        instructionToText(output, subInstruction, level + 1);
    }
}

bool LstScript::saveText(const std::string& fileOut)
{
    std::ofstream output(fileOut);
    if (!output.is_open()) {
        std::cerr << "Error: Cannot open file " << fileOut << std::endl;
        return false;
    }

    // Variables
    for (const std::string& var : d_ptr->m_listVariables) {
        output << var << std::endl;
    }

    // Warps
    for (const auto& warp : d_ptr->m_listWarps) {
        output << "Warp " << warp.second.name << std::endl;

        // Init block
        output << "\tInit" << std::endl;
        for (const LstScript::Instruction& instruction : warp.second.initBlock) {
            instructionToText(output, instruction, 2);
        }

        // Test blocks
        for (const auto& testBlock : warp.second.testBlockList) {
            output << "\tTest " << testBlock.first << std::endl;
            for (const LstScript::Instruction& instruction : testBlock.second) {
                instructionToText(output, instruction, 2);
            }
        }
    }

    // Subroutines
    for (const auto& subroutine : d_ptr->m_listSubroutines) {
        output << "Subroutine ";
        instructionToText(output, subroutine.second, 0);
    }

    output.close();

    return true;
}

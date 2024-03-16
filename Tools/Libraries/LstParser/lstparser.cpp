#include "lstparser.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

inline void trim(std::string& s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); })
                .base(),
        s.end());
}

std::vector<std::string> split(const std::string& s, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        trim(token);

        tokens.push_back(token);
    }
    return tokens;
}

/* Private */
class LstParser::LstParserPrivate {
    friend class LstParser;

public:
    bool nextLine(std::string& line);

    bool parseVariable(const std::string& line, std::string& varaiableName);
    bool parseWarp(const std::string& line, std::string& warpName);
    bool parseTest(const std::string& line, int& test);
    bool parsePlugin(const std::string& line, LstScript::Instruction& instruction);
    bool parseSubroutine(const std::string& line, LstScript::Instruction& instruction);

    bool parseInstruction(const std::string& line, LstScript::Instruction& instruction);
    bool parsePluginInstruction(const std::string& line, LstScript::Instruction& instruction);

private:
    std::ifstream m_file;
    int m_currentLine = 0;
};

bool LstParser::LstParserPrivate::nextLine(std::string& line)
{
    if (!std::getline(m_file, line)) {
        return false;
    }

    ++m_currentLine;

    // Lower case the line
    std::transform(line.begin(), line.end(), line.begin(), ::tolower);

    // Remove comments
    line = line.substr(0, line.find_first_of(';'));

    // Trim the line
    trim(line);

    // Skip empty lines
    if (line.empty()) {
        return nextLine(line);
    }

    return true;
}

bool LstParser::LstParserPrivate::parseVariable(const std::string& line, std::string& varaiableName)
{
    if (line.find("[bool]") != std::string::npos) {
        varaiableName = line.substr(line.find('=') + 1);
        trim(varaiableName);

        return true;
    }

    return false;
}

bool LstParser::LstParserPrivate::parseWarp(const std::string& line, std::string& warpName)
{
    if (line.find("[warp]") != std::string::npos) {
        warpName = line.substr(line.find('=') + 1, line.find(',') - line.find('=') - 1);
        trim(warpName);

        // Remove '.vr' if it exists
        if (warpName.find(".vr") != std::string::npos) {
            warpName = warpName.substr(0, warpName.find(".vr"));
        }

        return true;
    }

    return false;
}

bool LstParser::LstParserPrivate::parseTest(const std::string& line, int& test)
{
    if (line.find("[test]") != std::string::npos) {
        std::string testStr = line.substr(line.find('=') + 1);
        trim(testStr);

        // TODO: manage optional test number (e.g. [test]=6,1)
        test = std::stoi(testStr);

        if (test < -1) {
            std::cerr << m_currentLine << " Error: invalid test number: " << test << std::endl;
            return false;
        }

        return true;
    }

    return false;
}

bool LstParser::LstParserPrivate::parsePlugin(const std::string& line, LstScript::Instruction& instruction)
{
    if (line == "plugin") {
        instruction.name = "plugin";

        // Parse instructions
        std::string line;
        while (nextLine(line)) {
            if (line == "endplugin") {
                // End of plugin
                break;
            }

            LstScript::Instruction subInstruction;
            if (parsePluginInstruction(line, subInstruction)) {
                instruction.subInstructions.push_back(subInstruction);
                continue;
            }

            // Error
            std::cerr << m_currentLine << " Error: unknown line in plugin: " << line << std::endl;
            return false;
        }

        return true;
    }

    return false;
}

bool LstParser::LstParserPrivate::parseSubroutine(const std::string& line, LstScript::Instruction& instruction)
{
    if (line.find("label") != std::string::npos) {
        std::string labelName = line.substr(line.find(' ') + 1);

        instruction.name = labelName;

        // Parse instructions
        std::string line;
        while (nextLine(line)) {
            if (line == "return") {
                // End of subroutine
                break;
            }

            LstScript::Instruction subInstructionPlugin;
            if (parsePlugin(line, subInstructionPlugin)) {
                instruction.subInstructions.push_back(subInstructionPlugin);
                continue;
            }

            LstScript::Instruction subInstruction;
            if (parseInstruction(line, subInstruction)) {
                instruction.subInstructions.push_back(subInstruction);
                continue;
            }

            // Error
            std::cerr << m_currentLine << " Error: unknown line in subroutine: " << line << std::endl;
            return false;
        }

        return true;
    }

    return false;
}

bool LstParser::LstParserPrivate::parseInstruction(const std::string& line, LstScript::Instruction& instruction)
{
    std::string instructionName = line.substr(0, line.find_first_of(" ="));
    trim(instructionName);

    std::string params = line.substr(line.find_first_of(" =") + 1);
    trim(params);

    enum class InstrParamType {
        INT,
        DOUBLE,
        STRING
    };
    struct InstrTmp {
        std::string name;
        std::vector<InstrParamType> params;
        char separator = ',';
    };
    std::vector<InstrTmp> instrTmpList {
        //{"ifand", {InstrParamType::STRING}},
        //{"ifor", {InstrParamType::STRING}},
        { "gotowarp", { InstrParamType::STRING } },
        { "set", { InstrParamType::STRING, InstrParamType::INT }, '=' },
        { "playmusique", { InstrParamType::STRING, InstrParamType::INT } },
        { "stopmusique", { InstrParamType::STRING } },
        { "playsound", { InstrParamType::STRING, InstrParamType::INT, InstrParamType::INT } },
        { "stopsound", { InstrParamType::STRING } },
        { "playsound3d", { InstrParamType::STRING, InstrParamType::INT, InstrParamType::INT, InstrParamType::INT } },
        { "stopsound3d", { InstrParamType::STRING } },
        { "setcursor", { InstrParamType::STRING, InstrParamType::STRING, InstrParamType::INT } },
        { "setcursordefault", { InstrParamType::INT, InstrParamType::STRING } },
        { "hidecursor", { InstrParamType::STRING, InstrParamType::INT } },
        { "setangle", { InstrParamType::DOUBLE, InstrParamType::DOUBLE } },
        { "interpolangle", { InstrParamType::DOUBLE, InstrParamType::DOUBLE, InstrParamType::DOUBLE } },
        { "anglexmax", { InstrParamType::INT } },
        { "angleymax", { InstrParamType::INT } },
        { "return", {} },
        { "end", {} },
        { "fade", { InstrParamType::INT, InstrParamType::INT, InstrParamType::INT } },
        { "lockkey", { InstrParamType::INT, InstrParamType::STRING } }, // Second argument is either a string or a number (0)
        { "resetlockkey", {} },
        { "setzoom", { InstrParamType::STRING } },
        { "gosub", { InstrParamType::STRING } },
        { "not", { InstrParamType::STRING } }
    };

    if (instructionName == "ifand" || instructionName == "ifor") {
        std::string line;
        if (!nextLine(line)) {
            std::cerr << m_currentLine << " Error: unexpected end of file" << std::endl;
            return false;
        }

        LstScript::Instruction subInstruction;
        if (parsePlugin(line, subInstruction))
            ;
        else if (parseInstruction(line, subInstruction))
            ;
        else {
            // Error
            std::cerr << m_currentLine << " Error: unknown line in ifand/ifor: " << line << std::endl;
            return false;
        }

        std::vector<std::string> paramsList = split(params, ',');
        instruction.name = instructionName;
        for (const std::string& param : paramsList) {
            instruction.params.push_back(param);
        }
        instruction.subInstructions.push_back(subInstruction);

        return true;
    } else {
        for (const InstrTmp& instrTmp : instrTmpList) {
            if (instructionName == instrTmp.name) {
                instruction.name = instructionName;

                if (instrTmp.params.empty()) {
                    return true;
                }

                std::vector<std::string> paramsList = split(params, instrTmp.separator);
                if ((paramsList.size() != instrTmp.params.size())) {
                    std::cerr << m_currentLine << " Error: invalid " << instructionName << " instruction: " << params << std::endl;
                    return false;
                }

                for (size_t i = 0; i < paramsList.size(); ++i) {
                    if (instrTmp.params[i] == InstrParamType::INT) {
                        instruction.params.push_back(std::stoi(paramsList[i]));
                    } else if (instrTmp.params[i] == InstrParamType::DOUBLE) {
                        instruction.params.push_back(std::stod(paramsList[i]));
                    } else if (instrTmp.params[i] == InstrParamType::STRING) {
                        // Remove '.vr' if it exists
                        if (paramsList[i].find(".vr") != std::string::npos) {
                            paramsList[i] = paramsList[i].substr(0, paramsList[i].find(".vr"));
                        }

                        instruction.params.push_back(paramsList[i]);
                    }
                }

                return true;
            }
        }

        std::cerr << m_currentLine << " Error: unknown instruction: " << instructionName << std::endl;
        return false;
    }

    return true;
}

bool LstParser::LstParserPrivate::parsePluginInstruction(const std::string& line, LstScript::Instruction& instruction)
{
    // Line format: funName(var1, var2, var3, ...)

    // Find the function name
    std::string funName = line.substr(0, line.find('('));
    trim(funName);

    instruction.name = funName;

    // Parse parameters
    std::string params = line.substr(line.find('(') + 1, line.find(')') - line.find('(') - 1);

    std::vector<std::string> paramsList = split(params, ',');
    for (const std::string& param : paramsList) {
        try {
            instruction.params.push_back(std::stoi(param));
            continue;
        } catch (const std::invalid_argument&) {
            // Not an int: do nothing
        }

        try {
            instruction.params.push_back(std::stod(param));
            continue;
        } catch (const std::invalid_argument&) {
            // Not a double: do nothing
        }

        // String
        std::string paramTmp = param;
        // Remove '"' if it exists
        if (param.front() == '"' && param.back() == '"') {
            paramTmp = paramTmp.substr(1, paramTmp.size() - 2);
        }

        // Replace '\' by '/'
        std::replace(paramTmp.begin(), paramTmp.end(), '\\', '/');

        instruction.params.push_back(paramTmp);
    }

    return true;
}

/* Public */
LstParser::LstParser()
{
    d_ptr = new LstParserPrivate();
}

LstParser::~LstParser()
{
    delete d_ptr;
}

bool LstParser::parse(const std::string& filename, LstScript& lstScript)
{
    d_ptr->m_file.open(filename);
    if (!d_ptr->m_file) {
        std::cerr << "Could not open file: " << filename << "" << std::endl;
        return 1;
    }

    std::string currentWarp;
    int currentTest = -2;

    d_ptr->m_currentLine = 0;
    std::string line;
    while (d_ptr->nextLine(line)) {
        std::string var;
        if (d_ptr->parseVariable(line, var)) {
            lstScript.addVariable(var);
            continue;
        }

        if (d_ptr->parseWarp(line, currentWarp)) {
            continue;
        }

        if (d_ptr->parseTest(line, currentTest)) {
            if (currentWarp.empty()) {
                std::cerr << d_ptr->m_currentLine << " Error: [test] found before [warp]" << std::endl;
                return false;
            }

            continue;
        }

        LstScript::Instruction instructionPlugin;
        if (d_ptr->parsePlugin(line, instructionPlugin)) {
            if (currentWarp.empty()) {
                std::cerr << d_ptr->m_currentLine << " Error: plugin found before [warp]" << std::endl;
                return false;
            }

            lstScript.addInstruction(currentWarp, currentTest, instructionPlugin);
            continue;
        }

        LstScript::Instruction instructionSubroutine;
        if (d_ptr->parseSubroutine(line, instructionSubroutine)) {
            lstScript.addSubroutine(instructionSubroutine);
            continue;
        }

        LstScript::Instruction instruction;
        if (d_ptr->parseInstruction(line, instruction)) {
            if (currentWarp.empty()) {
                std::cerr << d_ptr->m_currentLine << " Error: instruction found before [warp]" << std::endl;
                return false;
            }

            lstScript.addInstruction(currentWarp, currentTest, instruction);
            continue;
        }

        std::cerr << d_ptr->m_currentLine << " Error: unknown line: " << line << std::endl;
        d_ptr->m_file.close();
        return false;
    }
    d_ptr->m_file.close();

    lstScript.optimize();

    return true;
}

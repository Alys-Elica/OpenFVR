#ifndef LSTPARSER_H
#define LSTPARSER_H

#include "liblstparser_globals.h"

#include <string>

#include "lstscript.h"

class LIBLSTPARSER_EXPORT LstParser {
public:
    LstParser();
    ~LstParser();

    bool parse(const std::string& filename, LstScript& lstScript);

private:
    class LstParserPrivate;
    LstParserPrivate* d_ptr;
};

#endif // LSTPARSER_H

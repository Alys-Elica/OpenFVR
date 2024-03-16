#ifndef FNXVR_H
#define FNXVR_H

#include <string>

class FnxVrPrivate;
class FnxVr {
public:
    FnxVr();
    ~FnxVr();

    bool isValid();

    bool loadFile(const std::string& vrFileName);
    bool loadTstFile(const std::string& tstFileName);
    bool loop();

private:
    FnxVrPrivate* d_ptr;
};

#endif // FNXVR_H

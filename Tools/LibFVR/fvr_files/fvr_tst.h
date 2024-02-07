#ifndef FVR_TST_H
#define FVR_TST_H

#include "libfvr_globals.h"

#include <string>

class FvrTstPrivate;
class LIBFVR_EXPORT FvrTst
{
public:
    FvrTst();
    ~FvrTst();

    bool loadFile(const std::string &fileName);

    int checkStaticZone(float x, float y);
    int checkAngleZone(float yaw, float pitch);

private:
    FvrTstPrivate *d_ptr;
};

#endif // FVR_TST_H

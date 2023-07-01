#ifndef FILEVR_H
#define FILEVR_H

#include <string>

#include "dct.h"
#include "file.h"
#include "frame.h"

class FileVr
{
public:
    enum class Type
    {
        VR_STATIC_PIC,
        VR_STATIC_VR,
        VR_UNKNOWN,
    };

    struct Animation
    {
        std::string name;
        int width;
        int height;
        std::vector<Frame> frameList;
    };

public:
    FileVr();
    ~FileVr();

    bool load(const std::string &filename);

    bool isValid() const;
    int getFrameWidth() const;
    int getFrameHeight() const;
    Type getType() const;

    bool getRawFrameRgb565(ByteArray &rgb565Data);
    bool getRawAnimationsRgba32(std::vector<Animation> &animationList);

private:
    bool readAnimation(Animation &animation);
    bool readAnimationFrame(Frame &frame);

private:
    FileVr::Type m_type;
    File m_file;
    Dct m_dct;
};

#endif // FILEVR_H

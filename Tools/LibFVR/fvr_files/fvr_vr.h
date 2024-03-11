#ifndef FVR_VR_H
#define FVR_VR_H

#include "libfvr_globals.h"

#include <string>

#include "fvr/image.h"

class LIBFVR_EXPORT FvrVr final
{
public:
    enum class Type
    {
        VR_STATIC_PIC,
        VR_STATIC_VR,
        VR_UNKNOWN,
    };

    struct AnimationBlock
    {
        uint32_t offset;
    };

    struct AnimationFrame
    {
        std::vector<uint32_t> offsetList;
        std::vector<uint8_t> data;
    };

    struct Animation
    {
        std::string name;
        std::vector<FvrVr::AnimationFrame> frames;
    };

public:
    FvrVr();
    ~FvrVr();

    FvrVr(const FvrVr &other) = delete;
    FvrVr &operator=(const FvrVr &other) = delete;

    bool open(const std::string &vrFileName);
    void close();
    bool isOpen() const;

    int getWidth() const;
    int getHeight() const;
    Type getType() const;

    bool getImage(Image &image) const;
    bool getImageCubemap(Image &image) const;

    const std::vector<uint8_t> &getData() const;

    // Animation data
    std::vector<std::string> getAnimationList() const;
    bool getAnimation(const std::string &animName, Animation &animation) const;

private:
    class FvrVrPrivate;
    FvrVrPrivate *d_ptr;
};

#endif // FVR_VR_H

#ifndef FVR_VR_H
#define FVR_VR_H

#include "libfvr_globals.h"

#include <string>

#include "fvr/image.h"

class FvrVrPrivate;
class LIBFVR_EXPORT FvrVr final
{
public:
    enum class Type
    {
        VR_STATIC_PIC,
        VR_STATIC_VR,
        VR_UNKNOWN,
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

private:
    FvrVrPrivate *d_ptr;
};

#endif // FVR_VR_H

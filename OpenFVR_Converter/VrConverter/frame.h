#ifndef FRAME_H
#define FRAME_H

#include <string>
#include <vector>

typedef std::vector<uint8_t> ByteArray;

class Frame
{
public:
    static Frame rgb565ToFrame(
        const ByteArray &rgb565Data,
        int width,
        int height);
    static bool vrToCubicMap(
        const Frame &inFrame,
        Frame &outFrame);
    static bool framesToWebP(
        const std::vector<Frame> &frameList,
        const std::string &filename);

public:
    Frame(int width = 0, int height = 0);
    ~Frame();

    bool isValid() const;

    int width() const;
    int height() const;

    bool resize(int width, int height);
    void clear();

    bool saveToWebp(const std::string &filename) const;
    bool saveToPng(const std::string &filename) const;

    const uint8_t *data() const;
    uint8_t *data();

private:
    int m_width;
    int m_height;
    ByteArray m_data;
};

#endif // FRAME_H

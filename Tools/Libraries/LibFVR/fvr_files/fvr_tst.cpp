#include "fvr_tst.h"

#include <iostream>
#include <vector>

#include <fvr/file.h>

/* Private */
class FvrTst::FvrTstPrivate {
    friend class FvrTst;

public:
    struct Zone {
        uint32_t index;
        float x1;
        float x2;
        float y1;
        float y2;
    };

private:
    std::vector<Zone> m_listZone;
};

/* Public */
FvrTst::FvrTst()
{
    d_ptr = new FvrTstPrivate();
}

FvrTst::~FvrTst()
{
    delete d_ptr;
}

bool FvrTst::loadFile(const std::string& fileName)
{
    d_ptr->m_listZone.clear();

    File tstFile;
    tstFile.setEndian(std::endian::little);
    if (!tstFile.open(fileName, std::ios::binary | std::ios::in)) {
        std::cerr << "Failed to open TST file" << std::endl;
        return false;
    }

    uint32_t zoneCount = 0;
    tstFile >> zoneCount;

    for (uint32_t i = 0; i < zoneCount; i++) {
        // TODO: some more checks ?
        FvrTstPrivate::Zone zone;

        zone.index = i;

        tstFile >> zone.x1;
        tstFile >> zone.x2;
        tstFile >> zone.y1;
        tstFile >> zone.y2;

        d_ptr->m_listZone.push_back(zone);
    }

    return true;
}

int FvrTst::checkStaticZone(float x, float y)
{
    for (const auto& zone : d_ptr->m_listZone) {
        float minX = std::min(zone.x1, zone.x2);
        float maxX = std::max(zone.x1, zone.x2);

        float minY = std::min(zone.y1, zone.y2);
        float maxY = std::max(zone.y1, zone.y2);

        if ((minX <= x) && (x <= maxX) && (minY <= y) && (y <= maxY)) {
            return zone.index;
        }
    }

    return -1;
}

int FvrTst::checkAngleZone(float yaw, float pitch)
{
    for (const auto& zone : d_ptr->m_listZone) {
        float minX = std::min(zone.x1, zone.x2);
        float maxX = std::max(zone.x1, zone.x2);
        if (3.141593f < maxX - minX) {
            float tmpX = maxX;
            maxX = minX + 6.283185f;
            minX = tmpX;
        }

        float minY = std::min(zone.y1, zone.y2);
        float maxY = std::max(zone.y1, zone.y2);
        if (3.141593f < maxY - minY) {
            float tmpY = maxY;
            maxY = minY + 6.283185f;
            minY = tmpY;
        }

        bool inX1 = (minX <= yaw) && (yaw <= maxX);
        bool inX2 = (minX <= yaw + 6.283185f) && (yaw + 6.283185f <= maxX);
        bool inY1 = (minY <= pitch) && (pitch <= maxY);
        bool inY2 = (minY <= pitch + 6.283185f) && (pitch + 6.283185f <= maxY);

        if ((inX1 || inX2) && (inY1 || inY2)) {
            return zone.index;
        }
    }

    return -1;
}

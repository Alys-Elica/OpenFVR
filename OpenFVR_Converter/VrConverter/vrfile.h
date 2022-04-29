#ifndef VRFILE_H
#define VRFILE_H

#include <cstdint>

#include <QSize>
#include <QString>
#include <QByteArray>

enum VrType {
    VR_TYPE0 = 0,
    VR_STATIC_PIC = 1,
    VR_STATIC_VR = 2,
    VR_UNKN = -1
};

class VrFile
{
public:
    VrFile(const QString &fileName);

private:
    bool parseVrFile(const QString &fileName, const QByteArray &data);
    VrType getVrType(int32_t header, int32_t type);

    void parseImage(const QByteArray &data, const QSize &imgDef);
    void parsePart2(const QByteArray &data, const QString &filePrefix, const QSize &imgDef);
};

#endif // VRFILE_H

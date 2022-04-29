#ifndef PAKTOOLS_H
#define PAKTOOLS_H

#include <QByteArray>
#include <QString>

struct PakSubFile {
    QString fileName;
    QByteArray data;
};

class PakTools
{
public:
    static QVector<PakSubFile> unpack(const QByteArray &in);
    static QByteArray pack(const QVector<PakSubFile> &files);

private:
    static QByteArray uncompressPakData3(const QByteArray &in);
    static QByteArray compressPakData3(const QByteArray &in);
};

#endif // PAKTOOLS_H

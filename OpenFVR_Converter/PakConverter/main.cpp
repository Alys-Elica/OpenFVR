#include <QDir>

#include "paktools.h"

int main(int argc, char *argv[])
{
    if (argc == 3) {
        QFile in(argv[2]);

        if (!in.open(QFile::ReadOnly)) {
            qDebug() << in.errorString();
            return 1;
        }

        QByteArray data = in.readAll();

        in.close();
        PakTools pak;

        if (QString(argv[1]) == "pack") {
            // TODO: implement if useful
        } else if (QString(argv[1]) == "unpack") {
            QVector<PakSubFile> files = pak.unpack(data);

            for (const PakSubFile &file : files) {
                QFile out(file.fileName);
                if (out.open(QFile::WriteOnly)) {
                    out.write(file.data);
                    out.close();
                } else {
                    qDebug() << out.errorString();
                }
            }
        } else {
            return 2;
        }
    }

    return 0;
}

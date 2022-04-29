#include "vrfile.h"

#include <QDataStream>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QPainter>

#include "dct.h"
#include "vrhuffman.h"

VrFile::VrFile(const QString &fileName)
{
    QFile file(fileName);

    if (file.open(QFile::ReadOnly)) {
        parseVrFile(fileName, file.readAll());
    }
}

VrType VrFile::getVrType(int32_t header, int32_t type)
{
    if (header != 0x12FA84AB) {
        qDebug() << "Header is invalid";
        return VR_UNKN;
    }

    if (type == -0x5f4e3e00)
        return VR_STATIC_VR;
    else if (type == -0x5f4e3c00)
        return VR_STATIC_PIC;
    return VR_TYPE0; // ???
}

bool VrFile::parseVrFile(const QString &fileName, const QByteArray &data)
{
    qDebug() << fileName;

    QDataStream ds(data);
    ds.setByteOrder(QDataStream::LittleEndian);

    int32_t header;
    int32_t fileSize;
    int32_t type;

    ds >> header;
    ds >> fileSize;
    ds >> type;

    VrType vrType = getVrType(header, type);
    if (fileSize != data.size()) {
        qDebug() << "Size is invalid";
        return false;
    }
    if (vrType == VR_UNKN) {
        qDebug() << "Invalid VR file";
        return false;
    }

    QSize imageSize(640, 480);
    if (vrType == VR_STATIC_VR) {
        imageSize = QSize(256, 6144);
    }

    /* PART 1 */
    uint32_t part1Size;
    ds >> part1Size;

    QByteArray imageData(part1Size, 0x00);
    ds.readRawData(imageData.data(), part1Size);

    QString prefix = fileName;
    prefix.replace(".vr", "", Qt::CaseInsensitive);
    parseImage(imageData, imageSize);

    /* PART 2 */
    // TODO: finish when DCT working
    /*while (!ds.atEnd()) {
        uint32_t part2Head;

        ds >> part2Head;

        if (part2Head == 0xa0b1c201) {
            uint32_t part2Size;
            ds >> part2Size;

            QByteArray part2Data(part2Size, 0x00);
            ds.readRawData(part2Data.data(), part2Size);

            parsePart2(part2Data, prefix, imageSize);
        }
    }

    if (!ds.atEnd()) {
        qDebug() << "Remaining data in VR file";
    }*/

    return true;
}

void VrFile::parseImage(const QByteArray &data, const QSize &imgDef)
{
    QDataStream ds(data);
    ds.setByteOrder(QDataStream::LittleEndian);

    // Base data
    uint32_t quality;
    uint32_t dataSize;

    ds >> quality;
    ds >> dataSize; // File size - 16

    // AC codes
    uint32_t acCodeSizeComp;
    uint32_t acCodeSizeUncomp;

    ds >> acCodeSizeComp;
    ds >> acCodeSizeUncomp;

    uint8_t *acCodeComp = new uint8_t[acCodeSizeComp];
    uint8_t *acCode = new uint8_t[acCodeSizeUncomp];

    ds.readRawData((char*)acCodeComp, acCodeSizeComp);

    VrHuffman huff;
    int tmpSize = huff.uncompress(acCodeSizeComp, acCodeSizeUncomp, acCodeComp, acCode);
    if (acCodeSizeUncomp != tmpSize) {
        qDebug() << acCodeSizeUncomp << tmpSize;
    }

    // AC data
    uint32_t acDataSize;

    ds >> acDataSize;

    uint8_t *acData = new uint8_t[acDataSize];
    ds.readRawData((char*)acData, acDataSize);

    // DC data
    uint32_t dcDataSize;

    ds >> dcDataSize;

    uint8_t *dcData = new uint8_t[dcDataSize];
    ds.readRawData((char*)dcData, dcDataSize);

    // Unpack image
    DCT dct(imgDef.width(),
            imgDef.height(),
            quality,
            dcData,
            acCode,
            acData
            );

    QImage img = dct.unpackImage();

    img.save("out.png");

    delete[] acCodeComp;
    delete[] acCode;
    delete[] acData;
    delete[] dcData;
}

void VrFile::parsePart2(const QByteArray &data, const QString &filePrefix, const QSize &imgDef)
{
    QDataStream ds(data);
    ds.setByteOrder(QDataStream::LittleEndian);

    char subNameTmp[0x20];
    ds.readRawData(subNameTmp, 0x20);
    QString subName(subNameTmp);

    QString subFolder = filePrefix + "/" + subName + "/";
    QDir dir;
    if (!dir.exists(subFolder)) {
        dir.mkpath(subFolder);
    }

    QFile part2File(subFolder + "part2.bin");
    part2File.open(QFile::WriteOnly);
    part2File.write(data);
    part2File.close();

    uint32_t subCount;
    ds >> subCount; // Frame count ?

    for (int i = 0; i < subCount; ++i) {
        uint32_t head;
        ds >> head;

        if (head == 0xa0b1c211) {
            uint32_t subSize;
            ds >> subSize;

            if (subSize <= 8) {
                qDebug() << "Empty frame";
                continue;
            }

            // Unknown elements
            uint32_t elementCount;
            ds >> elementCount;

            QByteArray elemData(elementCount * 4, 0x00);
            ds.readRawData(elemData.data(), elementCount * 4);

            QString subSubFolder = filePrefix + "/" + subName + "/" + QString::number(i) + "/";
            QDir dir;
            if (!dir.exists(subSubFolder)) {
                dir.mkpath(subSubFolder);
            }
            QFile subElemsFile(subSubFolder + "elements.bin");
            if (subElemsFile.open(QFile::WriteOnly)) {
                subElemsFile.write(elemData);
                subElemsFile.close();
            }

            // Sub-image
            //parseImage(ds, subSubFolder, imgDef);
        }
    }
}

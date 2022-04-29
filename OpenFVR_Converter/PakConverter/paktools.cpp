#include "paktools.h"

#include <bitset>

#include <QDataStream>
#include <QDebug>

QVector<PakSubFile> PakTools::unpack(const QByteArray &in)
{
    QVector<PakSubFile> uncompressedFiles;

    QDataStream ds(in);

    ds.setByteOrder(QDataStream::LittleEndian);

    char header[5] = { 0, 0, 0, 0, 0 };
    uint32_t fileSize;

    ds.readRawData(header, 4);
    ds >> fileSize;

    while (!ds.atEnd()) {
        char compFileName[16];          // Compressed file name
        uint32_t compressionLevel;
        uint32_t compressedSize;        // Compressed file size
        uint32_t uncompressedSize;      // Compressed file originalSize (uncompressed)
        ds.readRawData(compFileName, 16);
        ds >> compressionLevel;
        ds >> compressedSize;
        ds >> uncompressedSize;

        QByteArray compressedData(compressedSize, 0x00);
        ds.readRawData(compressedData.data(), compressedSize);

        PakSubFile subFile;

        subFile.fileName = QString(compFileName);

        switch (compressionLevel) {
        case 3: {
            subFile.data = uncompressPakData3(compressedData);
            break;
        }

        default:
            qDebug() << "Compression not yet known";
            break;
        }

        uncompressedFiles.push_back(subFile);
    }

    return uncompressedFiles;
}

QByteArray PakTools::pack(const QVector<PakSubFile> &files)
{
    if (files.isEmpty()) {
        return "";
    }

    QByteArray compressedFiles;
    for (const PakSubFile &file : files) {
        if (file.data.isEmpty() || file.fileName.isEmpty() || file.fileName.size() > 15) {
            continue;
        }

        QByteArray compressedData = compressPakData3(file.data);

        QByteArray fileData;
        QDataStream dsSubFile(&fileData, QDataStream::WriteOnly);

        dsSubFile.setByteOrder(QDataStream::LittleEndian);

        char compFileName[16];
        for (int i = 0; i < 16; ++i) {
            if (i < file.fileName.size()) {
                compFileName[i] = file.fileName[i].toLatin1();
            } else {
                compFileName[i] = 0x00;
            }
        }
        dsSubFile.writeRawData(compFileName, 16);
        dsSubFile << (uint32_t)3;
        dsSubFile << (uint32_t)compressedData.size();
        dsSubFile << (uint32_t)file.data.size();
        dsSubFile.writeRawData(compressedData.data(), compressedData.size());

        compressedFiles.append(fileData);
    }

    QByteArray out;
    QDataStream ds(&out, QDataStream::WriteOnly);

    ds.setByteOrder(QDataStream::LittleEndian);

    char header[5] = "PAKF";

    ds.writeRawData(header, 4);
    ds << (uint32_t)compressedFiles.size() + 8;
    ds.writeRawData(compressedFiles.data(), compressedFiles.size());

    return out;
}

QByteArray PakTools::uncompressPakData3(const QByteArray &in)
{
    qDebug() << "Starting PAK file uncompression";

    QByteArray out;
    QDataStream ds(in);
    ds.setByteOrder(QDataStream::BigEndian);
    while (!ds.atEnd()) {
        uint8_t byte;
        ds >> byte;

        if (std::bitset<8>(byte)[7]) {
            /*
             * If most significant bit is set to 1
             *      => copy of N1 bytes from already uncompressed data starting at N2 bytes from the end of
             *         current outputed data
             *              N1 = byte & 0x3f;
             *              N2 = index (1 byte if second most significant bit is 1, 2 bytes if 0)
            */

            int size = (byte & 0x3f) + 1;
            int index;
            if (std::bitset<8>(byte)[6]) {
                uint8_t tmpIndex;
                ds >> tmpIndex;
                index = out.size() - (tmpIndex + 1);
            } else {
                uint16_t tmpIndex;
                ds >> tmpIndex;
                index = out.size() - (tmpIndex + 1);
            }
            for (int i = 0; i < size; ++i) {
                out.append(out[index++]);
            }
        } else {
            QByteArray tmp(byte + 1, 0x00);
            ds.readRawData(tmp.data(), byte + 1);
            out.append(tmp);
        }
    }

    return out;
}

QByteArray PakTools::compressPakData3(const QByteArray &in)
{
    qDebug() << "Starting PAK file compression";

    QByteArray out;

    if (in.size() < 4) {
        return in;
    }

    int currentIndex = 0;
    while (currentIndex < in.size()) {
        // Check if previous sequence can be used
        QList<QPair<uint32_t, uint8_t> > sequences;
        int backtraceIndex = std::max(0, currentIndex - 65535);
        while (backtraceIndex < currentIndex) {
            int i = 0;
            while ((currentIndex + i < in.size()) && (in[backtraceIndex + i] == in[currentIndex + i]) && (i < 64)) {
                ++i;
            }
            if ((currentIndex - backtraceIndex <= 256 && i > 2) || ((currentIndex - backtraceIndex > 256 && i > 3))) {
                sequences.append({currentIndex - backtraceIndex, i});
            }
            ++backtraceIndex;
        }

        // Manage potential sequences
        if (sequences.isEmpty()) {
            // If no usable previous sequence found => take the 'size' next bytes until a sequence can be used
            int size = 0;
            bool sequenceFound = false;
            while (!sequenceFound) {
                ++size;
                int nextIndex = currentIndex + size;
                int backtraceIndex = std::max(0, nextIndex - 65535);
                while (backtraceIndex < nextIndex) {
                    int i = 0;
                    while ((nextIndex + i < in.size()) && (in[backtraceIndex + i] == in[nextIndex + i]) && (i < 64)) {
                        ++i;
                    }
                    if (i > 2) {
                        sequenceFound = true;
                        break;
                    }
                    ++backtraceIndex;
                }
                if (currentIndex + size >= in.size()) {
                    break;
                }
            }

            out.append((uint8_t)size - 1);
            out.append(in.mid(currentIndex, size));
            currentIndex += size;
        } else {
            // If previous sequences are found => use the most efficient one
            QPair<uint32_t, uint8_t> bestSequence(0, 0);
            for (const QPair<uint32_t, uint8_t> &seq : sequences) {
                if ((seq.second > bestSequence.second) || (seq.second == bestSequence.second && seq.first < bestSequence.first)) {
                    bestSequence = seq;
                }
            }

            uint8_t ctrlByte = 0b10000000;
            if (bestSequence.first < 256) {
                ctrlByte += 0b01000000;
            }
            ctrlByte += bestSequence.second;

            out.append(ctrlByte - 1);

            bestSequence.first--;
            if (bestSequence.first < 255) {
                out.append(bestSequence.first & 0xff);
            } else {
                out.append((bestSequence.first >> 8) & 0xff);
                out.append(bestSequence.first & 0xff);
            }

            currentIndex += bestSequence.second;
        }
    }

    return out;
}

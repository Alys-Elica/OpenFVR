#ifndef DCT_H
#define DCT_H

#include <cstdint>

#include <QImage>

class DCT
{
public:
    DCT(int width, int height, int quality, uint8_t *dcData, uint8_t *acCode, uint8_t *acData);

    QImage unpackImage();

private:
    void init();

    void getBlock(int block[3][64]);

    bool unpackBlock(int block[64]);
    int readDcBits(const int count);
    int readAcBits(const int count);

    static void zigzag(int block[64]);
    static void idct(int block[64]);
    static void ycbcrToRgb(int block[3][64]);

    void setQuality(int quality);
    void getDequantTable(const int baseTable[64], int destTable[64]);
    void inverseNormaliseQuant(int block[64]);

    int m_dcIndex;
    uint8_t m_dcBitmask;

    int m_acCodeIndex;

    int m_acDataIndex;
    uint8_t m_acDataBitmask;

    int m_quality;
    int m_width;
    int m_height;
    uint8_t* m_dcData;
    uint8_t* m_acCode;
    uint8_t* m_acData;

    int m_lumaDequant[64];
    int m_chromaDequant[64];
};

#endif // DCT_H

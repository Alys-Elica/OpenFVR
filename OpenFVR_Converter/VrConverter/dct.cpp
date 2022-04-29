/*
 * VR files DCT compression is similar to the 4XM video format one
 * Part of this code come from FFmpeg 4XM codec by Michael Niedermayer (released under LGPL)
 */

#include "dct.h"

#include <cstring>


// Defines come from FFmpeg 4XM codec by Michael Niedermayer (released under LGPL)
#define FIX_1_082392200  70936
#define FIX_1_414213562  92682
#define FIX_1_847759065 121095
#define FIX_2_613125930 171254

#define MULTIPLY(var, const) ((int)((var) * (unsigned)(const)) >> 16)


DCT::DCT(int width, int height, int quality, uint8_t *dcData, uint8_t *acCode, uint8_t *acData)
{
    // Base init
    m_width = width;
    m_height = height;
    m_dcData = dcData;
    m_acCode = acCode;
    m_acData = acData;

    // Inverse init
    setQuality(quality);
}

QImage DCT::unpackImage()
{
    init();

    QImage img(m_width, m_height, QImage::Format_RGB32);

    int horizontalBlockCount = m_width / 8;
    int verticalBlockCount = m_height / 8;
    int blockCount = horizontalBlockCount * verticalBlockCount;

    for (int blockIdx = 0; blockIdx < blockCount; ++blockIdx) {
        int block[3][64];

        getBlock(block);

        int xBeg = (blockIdx % horizontalBlockCount) * 8;
        int yBeg = (blockIdx / horizontalBlockCount) * 8;

        ycbcrToRgb(block);

        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                QColor pix(block[0][x + y * 8], block[1][x + y * 8], block[2][x + y * 8]);
                img.setPixel(x + xBeg, y + yBeg, pix.rgb());
            }
        }
    }

    return img;
}

void DCT::init()
{
    m_dcIndex = 0;
    m_dcBitmask = 0x80;

    m_acCodeIndex = 0;

    m_acDataIndex = 0;
    m_acDataBitmask = 0x80;
}

void DCT::getBlock(int block[3][64])
{
    for (int i = 0; i < 3; ++i) {
        unpackBlock(block[i]);
        zigzag(block[i]);
    }

    // Dequantization
    for (int i = 0; i < 64; ++i) {
        block[0][i] *= m_lumaDequant[i];
        block[1][i] *= m_chromaDequant[i];
        block[2][i] *= m_chromaDequant[i];
    }

    // Inverse DCT
    for (int i = 0; i < 3; ++i) {
        idct(block[i]);
    }
}

bool DCT::unpackBlock(int block[64])
{
    for (int i = 0; i < 64; ++i) {
        block[i] = 0;
    }

    block[0] = readDcBits(8);

    for (int idx = 1; idx < 64;) {
        uint8_t acCode = m_acCode[m_acCodeIndex++];
        if (acCode == 0) {
            break;
        } else if (acCode == 0xf0) {
            idx += 16;
        } else {
            int offset = acCode >> 4;
            int size = acCode & 0xf;
            if (size == 0) {
                // 0 coeff
                return false;
            }

            uint32_t level = readAcBits(size);

            // TODO: unserstand this part
            if ((level & (1 << (size - 1))) == 0) {
                level += (1 - (1 << size));
            }
            // TODO

            idx += offset;
            if (idx >= 64) {
                return true;
            }

            block[idx++] = level;
        }
    }

    return true;
}

int DCT::readDcBits(const int count)
{
    int byte = 0;

    uint32_t bit = 1;

    for (int i = 0; i < count; ++i) {
        if ((m_dcData[m_dcIndex] & m_dcBitmask) != 0) {
            byte |= bit;
        }
        bit <<= 1;

        m_dcBitmask >>= 1;
        if (m_dcBitmask == 0) {
            m_dcBitmask = 0x80;
            ++m_dcIndex;
        }
    }

    return byte;
}

int DCT::readAcBits(const int count)
{
    int byte = 0;

    uint32_t bit = 1;

    for (int i = 0; i < count; ++i) {
        if ((m_acData[m_acDataIndex] & m_acDataBitmask) != 0) {
            byte |= bit;
        }
        bit <<= 1;

        m_acDataBitmask >>= 1;
        if (m_acDataBitmask == 0) {
            m_acDataBitmask = 0x80;
            ++m_acDataIndex;
        }
    }

    return byte;
}

void DCT::zigzag(int block[64])
{
    const uint8_t zigzag[64] = {
        0,   1,  8, 16,  9,  2,  3, 10,
        17, 24, 32, 25, 18, 11,  4,  5,
        12, 19, 26, 33, 40, 48, 41, 34,
        27, 20, 13,  6,  7, 14, 21, 28,
        35, 42, 49, 56, 57, 50, 43, 36,
        29, 22, 15, 23, 30, 37, 44, 51,
        58, 59, 52, 45, 38, 31, 39, 46,
        53, 60, 61, 54, 47, 55, 62, 63
    };

    int tmp[64];

    for (int i = 0; i < 64; ++i) {
        tmp[zigzag[i]] = block[i];
    }

    for (int i = 0; i < 64; ++i) {
        block[i] = tmp[i];
    }
}

// Comes from FFmpeg 4XM codec by Michael Niedermayer (released under LGPL)
void DCT::idct(int block[64])
{
    int tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
    int tmp10, tmp11, tmp12, tmp13;
    int z5, z10, z11, z12, z13;
    int i;
    int temp[64];

    for (i = 0; i < 8; i++) {
        tmp10 = block[8 * 0 + i] + block[8 * 4 + i];
        tmp11 = block[8 * 0 + i] - block[8 * 4 + i];

        tmp13 = block[8 * 2 + i] + block[8 * 6 + i];
        tmp12 = MULTIPLY(block[8 * 2 + i] - block[8 * 6 + i], FIX_1_414213562) - tmp13;

        tmp0 = tmp10 + tmp13;
        tmp3 = tmp10 - tmp13;
        tmp1 = tmp11 + tmp12;
        tmp2 = tmp11 - tmp12;

        z13 = block[8 * 5 + i] + block[8 * 3 + i];
        z10 = block[8 * 5 + i] - block[8 * 3 + i];
        z11 = block[8 * 1 + i] + block[8 * 7 + i];
        z12 = block[8 * 1 + i] - block[8 * 7 + i];

        tmp7  =          z11 + z13;
        tmp11 = MULTIPLY(z11 - z13, FIX_1_414213562);

        z5    = MULTIPLY(z10 + z12, FIX_1_847759065);
        tmp10 = MULTIPLY(z12,  FIX_1_082392200) - z5;
        tmp12 = MULTIPLY(z10, -FIX_2_613125930) + z5;

        tmp6 = tmp12 - tmp7;
        tmp5 = tmp11 - tmp6;
        tmp4 = tmp10 + tmp5;

        temp[8 * 0 + i] = tmp0 + tmp7;
        temp[8 * 7 + i] = tmp0 - tmp7;
        temp[8 * 1 + i] = tmp1 + tmp6;
        temp[8 * 6 + i] = tmp1 - tmp6;
        temp[8 * 2 + i] = tmp2 + tmp5;
        temp[8 * 5 + i] = tmp2 - tmp5;
        temp[8 * 4 + i] = tmp3 + tmp4;
        temp[8 * 3 + i] = tmp3 - tmp4;
    }

    for (i = 0; i < 8 * 8; i += 8) {
        tmp10 = temp[0 + i] + temp[4 + i];
        tmp11 = temp[0 + i] - temp[4 + i];

        tmp13 = temp[2 + i] + temp[6 + i];
        tmp12 = MULTIPLY(temp[2 + i] - temp[6 + i], FIX_1_414213562) - tmp13;

        tmp0 = tmp10 + tmp13;
        tmp3 = tmp10 - tmp13;
        tmp1 = tmp11 + tmp12;
        tmp2 = tmp11 - tmp12;

        z13 = temp[5 + i] + temp[3 + i];
        z10 = temp[5 + i] - temp[3 + i];
        z11 = temp[1 + i] + temp[7 + i];
        z12 = temp[1 + i] - temp[7 + i];

        tmp7  = z11 + z13;
        tmp11 = MULTIPLY(z11 - z13, FIX_1_414213562);

        z5    = MULTIPLY(z10 + z12, FIX_1_847759065);
        tmp10 = MULTIPLY(z12,  FIX_1_082392200) - z5;
        tmp12 = MULTIPLY(z10, -FIX_2_613125930) + z5;

        tmp6 = tmp12 - tmp7;
        tmp5 = tmp11 - tmp6;
        tmp4 = tmp10 + tmp5;

        block[0 + i] = (tmp0 + tmp7)/* >> 6*/;
        block[7 + i] = (tmp0 - tmp7)/* >> 6*/;
        block[1 + i] = (tmp1 + tmp6)/* >> 6*/;
        block[6 + i] = (tmp1 - tmp6)/* >> 6*/;
        block[2 + i] = (tmp2 + tmp5)/* >> 6*/;
        block[5 + i] = (tmp2 - tmp5)/* >> 6*/;
        block[4 + i] = (tmp3 + tmp4)/* >> 6*/;
        block[3 + i] = (tmp3 - tmp4)/* >> 6*/;
    }
}

void DCT::ycbcrToRgb(int block[3][64])
{
    // TODO: fix this thing
    for (int x = 0; x < 8; ++x) {
        for (int y = 0; y < 8; ++y) {
            int yVal = block[0][x + 8 * y] + 128;
            int cb = block[1][x + 8 * y];
            int cr = block[2][x + 8 * y];

            int R = yVal + cb * 2;
            int G = (int)((unsigned long long)((long long)cb * 0x55555555) >> 0x20) - cb;
            G = (((G >> 1) - (G >> 0x1f)) - (cr * 8) / 10) + yVal;
            int B = yVal + (cr << 4) / 10;

            /*R = std::max(0, std::min(255, R));
            G = std::max(0, std::min(255, G));
            B = std::max(0, std::min(255, B));*/

            block[0][x + 8 * y] = R >> 4;
            block[1][x + 8 * y] = G >> 4;
            block[2][x + 8 * y] = B >> 4;
        }
    }
}

const int lumaBaseDequant[64] = {
    16, 11, 10, 16,  24,  40,  51,  61,
    12, 12, 14, 19,  26,  58,  60,  55,
    14, 13, 16, 24,  40,  57,  69,  56,
    14, 17, 22, 29,  51,  87,  80,  62,
    18, 22, 37, 56,  68, 109, 103,  77,
    24, 35, 55, 64,  81, 104, 113,  92,
    49, 64, 78, 87, 103, 121, 120, 101,
    72, 92, 95, 98, 112, 100, 103,  99,
};

const int chromaBaseDequant[64] = {
    17, 18, 24, 47, 99, 99, 99, 99,
    18, 21, 26, 66, 99, 99, 99, 99,
    24, 26, 56, 99, 99, 99, 99, 99,
    47, 66, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99,
};

void DCT::setQuality(int quality)
{
    if (quality < 0) {
        m_quality = 5000;
        return;
    }

    if (quality < 0x65) {
        if (quality < 0x32) {
            m_quality = (int)(5000 / (long long)quality);
            return;
        }
    } else {
        quality = 100;
    }

    m_quality = (100 - quality) * 2;

    getDequantTable(lumaBaseDequant, m_lumaDequant);
    inverseNormaliseQuant(m_lumaDequant);
    getDequantTable(chromaBaseDequant, m_chromaDequant);
    inverseNormaliseQuant(m_chromaDequant);
}

void DCT::getDequantTable(const int baseTable[64], int destTable[64])
{
    for (int i = 0; i < 64; ++i) {
        int val = (baseTable[i] * m_quality + 0x32) / 100;
        if (val < 8) {
            val = 8;
        } else if (0xFF < val) {
            val = 0xFF;
        }
        destTable[i] = val;
    }
}

void DCT::inverseNormaliseQuant(int block[64])
{
    const int quantNorm[64] = {
        0x4000, 0x58C5, 0x539F, 0x4B42, 0x4000, 0x3249, 0x22A3, 0x11A8,
        0x58C5, 0x7B21, 0x73FC, 0x6862, 0x58C5, 0x45BF, 0x300B, 0x187E,
        0x539F, 0x73FC, 0x6D41, 0x6254, 0x539F, 0x41B3, 0x2D41, 0x1712,
        0x4B42, 0x6862, 0x6254, 0x587E, 0x4B42, 0x3B21, 0x28BA, 0x14C3,
        0x4000, 0x58C5, 0x539F, 0x4B42, 0x4000, 0x3249, 0x22A3, 0x11A8,
        0x3249, 0x45BF, 0x41B3, 0x3B21, 0x3249, 0x2782, 0x1B37, 0x0DE0,
        0x22A3, 0x300B, 0x2D41, 0x28BA, 0x22A3, 0x1B37, 0x12BF, 0x098E,
        0x11A8, 0x187E, 0x1712, 0x14C3, 0x11A8, 0x0DE0, 0x098E, 0x04DF,
    };

    for (int i = 0; i < 64; ++i) {
        block[i] = (quantNorm[i] * block[i]) >> 0xD;
    }
}

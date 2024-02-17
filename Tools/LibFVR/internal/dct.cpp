#include "dct.h"

#include <iostream>
#include <cstring>
#include <cmath>
#include <algorithm>

#include "fvr/datastream.h"
#include "fvr/huffmantable.h"
#include "fvr/inversebitstream.h"

const int AAN_scales[64] = {
    16384, 22725, 21407, 19266, 16384, 12873, 8867, 4520,
    22725, 31521, 29692, 26722, 22725, 17855, 12299, 6270,
    21407, 29692, 27969, 25172, 21407, 16819, 11585, 5906,
    19266, 26722, 25172, 22654, 19266, 15137, 10426, 5315,
    16384, 22725, 21407, 19266, 16384, 12873, 8867, 4520,
    12873, 17855, 16819, 15137, 12873, 10114, 6967, 3552,
    8867, 12299, 11585, 10426, 8867, 6967, 4799, 2446,
    4520, 6270, 5906, 5315, 4520, 3552, 2446, 1247};

const int Y_Q[64] = {
    16, 11, 10, 16, 24, 40, 51, 61,
    12, 12, 14, 19, 26, 58, 60, 55,
    14, 13, 16, 24, 40, 57, 69, 56,
    14, 17, 22, 29, 51, 87, 80, 62,
    18, 22, 37, 56, 68, 109, 103, 77,
    24, 35, 55, 64, 81, 104, 113, 92,
    49, 64, 78, 87, 103, 121, 120, 101,
    72, 92, 95, 98, 112, 100, 103, 99};

const int C_Q[64] = {
    17, 18, 24, 47, 99, 99, 99, 99,
    18, 21, 26, 66, 99, 99, 99, 99,
    24, 26, 56, 99, 99, 99, 99, 99,
    47, 66, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99};

const int ZIGZAG[64] = {
    0, 1, 8, 16, 9, 2, 3, 10,
    17, 24, 32, 25, 18, 11, 4, 5,
    12, 19, 26, 33, 40, 48, 41, 34,
    27, 20, 13, 6, 7, 14, 21, 28,
    35, 42, 49, 56, 57, 50, 43, 36,
    29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46,
    53, 60, 61, 54, 47, 55, 62, 63};

int VEC_COL[64] = {
    0, 8, 16, 24, 32, 40, 48, 56,
    1, 9, 17, 25, 33, 41, 49, 57,
    2, 10, 18, 26, 34, 42, 50, 58,
    3, 11, 19, 27, 35, 43, 51, 59,
    4, 12, 20, 28, 36, 44, 52, 60,
    5, 13, 21, 29, 37, 45, 53, 61,
    6, 14, 22, 30, 38, 46, 54, 62,
    7, 15, 23, 31, 39, 47, 55, 63};

int VEC_ROW[64] = {
    0, 1, 2, 3, 4, 5, 6, 7,
    8, 9, 10, 11, 12, 13, 14, 15,
    16, 17, 18, 19, 20, 21, 22, 23,
    24, 25, 26, 27, 28, 29, 30, 31,
    32, 33, 34, 35, 36, 37, 38, 39,
    40, 41, 42, 43, 44, 45, 46, 47,
    48, 49, 50, 51, 52, 53, 54, 55,
    56, 57, 58, 59, 60, 61, 62, 63};

void idctVectorTransform(int mcu[64], int *vec_lookup)
{
    int add_0_4, sub_0_4,
        add_1_7, sub_1_7,
        add_2_6, sub_2_6,
        add_5_3, sub_5_3,
        tmp0, tmp1,
        tmp2, tmp3,
        tmp4, mul5,
        mul1, mul2,
        mul3, mul4;

    int *vec = vec_lookup;

#define mul(a, b) (((a) * (b)) >> 16)

    for (int i = 0; i < 64; i += 8, vec += 8)
    {
        add_0_4 = mcu[vec[0]] + mcu[vec[4]];
        sub_0_4 = mcu[vec[0]] - mcu[vec[4]];
        add_1_7 = mcu[vec[1]] + mcu[vec[7]];
        sub_1_7 = mcu[vec[1]] - mcu[vec[7]];
        add_2_6 = mcu[vec[2]] + mcu[vec[6]];
        sub_2_6 = mcu[vec[2]] - mcu[vec[6]];
        add_5_3 = mcu[vec[5]] + mcu[vec[3]];
        sub_5_3 = mcu[vec[5]] - mcu[vec[3]];

        tmp0 = add_1_7 + add_5_3;
        tmp1 = add_2_6 + add_0_4;
        tmp2 = add_0_4 - add_2_6;

        mul1 = mul(92682, sub_2_6) - add_2_6;

        tmp3 = mul1 + sub_0_4;
        tmp4 = sub_0_4 - mul1;

        mul2 = mul(121095, sub_1_7 + sub_5_3);
        mul3 = mul2 + mul(-171254, sub_5_3) - tmp0;
        mul4 = mul(92682, add_1_7 - add_5_3) - mul3;
        mul5 = mul4 + mul(70936, sub_1_7) - mul2;

        mcu[vec[0]] = tmp1 + tmp0;
        mcu[vec[1]] = mul3 + tmp3;
        mcu[vec[2]] = mul4 + tmp4;
        mcu[vec[3]] = tmp2 - mul5;
        mcu[vec[4]] = mul5 + tmp2;
        mcu[vec[5]] = tmp4 - mul4;
        mcu[vec[6]] = tmp3 - mul3;
        mcu[vec[7]] = tmp1 - tmp0;
    }
#undef mul
}

void idct(int mcu[64])
{
    idctVectorTransform(mcu, VEC_COL);
    idctVectorTransform(mcu, VEC_ROW);

    // Performs descaling from previous upscaling, see AA&N scales.
    for (int i = 0; i < 64; i++)
    {
        mcu[i] = std::clamp(mcu[i] / 16, -128, 128);
    }
}

/* Private */
class DctPrivate
{
    friend class Dct;

private:
    int width;
    int height;
    int quality;
    int y_quants[64];
    int c_quants[64];

public:
    void setQuality(int quality)
    {
        quality = std::clamp(quality, 1, 100);

        if (quality >= 50)
        {
            this->quality = 2 * (100 - quality);
        }
        else
        {
            this->quality = 5000 / quality;
        }

        for (int i = 0; i < 64; ++i)
        {
            this->y_quants[i] = std::clamp((Y_Q[i] * this->quality + 50) / 100, 8, 255);
            this->y_quants[i] = (this->y_quants[i] * AAN_scales[i]) >> 13;

            this->c_quants[i] = std::clamp((C_Q[i] * this->quality + 50) / 100, 8, 255);
            this->c_quants[i] = (this->c_quants[i] * AAN_scales[i]) >> 13;
        }
    }

    void unpackMcu(InverseBitstream &ac, InverseBitstream &dc, DataStream &dsAcCode, int mcu[64], int quants[64])
    {
        off_t index = 0;
        mcu[ZIGZAG[index]] = dc.next(8) * quants[ZIGZAG[index]];

        while (++index < 64)
        {
            uint8_t acCode;
            dsAcCode >> acCode;

            if (acCode == 0)
            {
                while (index < 64)
                {
                    mcu[ZIGZAG[index++]] = 0;
                }

                break;
            }

            // Most significant nibble is number of null-bytes padding
            uint8_t ac_padd = acCode >> 4;
            for (int i = 0; i < ac_padd; i++)
            {
                mcu[ZIGZAG[index++]] = 0;
            }

            // Least significant is number of bits to read from AC.
            uint8_t ac_bits = acCode & 0xf;

            int ac_data;
            if (ac_bits > 0)
            {
                ac_data = ac.next(ac_bits);

                if (((1 << (ac_bits - 1)) & ac_data) == 0)
                {
                    ac_data += 1 - (1 << ac_bits);
                }
            }
            else
            {
                ac_data = 0;
            }

            mcu[ZIGZAG[index]] = quants[ZIGZAG[index]] * ac_data;
        }

        idct(mcu);
    }

    void ycrcb_to_rgb(
        int width,
        int y[64],
        int cb[64],
        int cr[64],
        int *r,
        int *g,
        int *b)
    {
        int index;
        int lum;

        for (int i = 0; i < 64; i++)
        {
            lum = (y[i] + 128) << 16;

            /*
             MCU's are unpacked as an 8x8 matrix from left -> right, top -> bottom.
             Until this point we have been working on a vector representation,
             this index tells us where in the image this pixel should be unpacked.

             If this is the first pixel, the index would be 0. If it's the 8th pixel it
             would be the first pixel on the next line, (i.e 640 if the picture is 640 pixels wide).
            */
            index = std::floor(i / 8) * width + i % 8;

            // TODO: This seems to be a source for cross-platform differences since different compilers
            // approach float to integer conversion differently. Should this be a proper ASM or some other mean
            // of rounding?
            r[index] = lum + cr[i] * 91881.47199999999;
            g[index] = lum + (cr[i] * -46801.87904) + (cb[i] * -22553.55904);
            b[index] = lum + cb[i] * 116129.792;
        }
    }

    void putBlock(uint16_t *buffer, int width, int *r, int *g, int *b)
    {
        int r_overflow = 0,
            g_overflow = 0,
            b_overflow = 0;

        int r_val,
            g_val,
            b_val;

        for (int i = 0; i < width * 8; i++)
        {
            r_val = std::clamp(r[i], 0, 0xff0000);
            g_val = std::clamp(g[i], 0, 0xff0000);
            b_val = std::clamp(b[i], 0, 0xff0000);

            buffer[i] = (((r_val >> 8) & 0xF800) +
                         ((g_val >> 13) & 0xFFE0) +
                         (b_val >> 19));

            r_overflow = (r_val - (r_val & 0xF80000)) >> 1;
            g_overflow = (g_val - (g_val & 0xFC0000)) >> 1;
            b_overflow = (b_val - (b_val & 0xF80000)) >> 1;

            if ((i + 1) % width > 0)
            {
                r[i + 1] += r_overflow;
                g[i + 1] += g_overflow;
                b[i + 1] += b_overflow;

                r[width + i] += r_overflow;
                g[width + i] += g_overflow;
                b[width + i] += b_overflow;
            }
        }
    }
};

/* Public */
Dct::Dct()
{
    d_ptr = new DctPrivate();
}

Dct::~Dct()
{
    delete d_ptr;
}

bool Dct::unpack(
    const std::vector<uint8_t> &imageData,
    const int quality,
    const int width,
    const int height,
    std::vector<uint8_t> &rgb565Data)
{
    rgb565Data.resize(width * height * 2);

    d_ptr->setQuality(quality);

    // Unpack
    d_ptr->width = width;
    d_ptr->height = height;

    DataStream ds;
    ds.setData(imageData);
    ds.setEndian(std::endian::little);

    // AC code data
    uint32_t acCodeCompSize;
    uint32_t acCodeUncompSize;

    ds >> acCodeCompSize;
    ds >> acCodeUncompSize;

    std::vector<uint8_t> dataAcCodeComp;
    std::vector<uint8_t> dataAcCode;
    ds.read(dataAcCodeComp, acCodeCompSize);

    HuffmanTable::decompress(dataAcCodeComp, dataAcCode, acCodeUncompSize);

    // AC data
    uint32_t acSize;
    ds >> acSize;

    std::vector<uint8_t> dataAc;
    ds.read(dataAc, acSize);

    // DC data
    uint32_t dcSize;
    ds >> dcSize;

    std::vector<uint8_t> dataDc;
    ds.read(dataDc, dcSize);

    // Unpack
    InverseBitstream ac;
    ac.setData(dataAc);

    InverseBitstream dc;
    dc.setData(dataDc);

    DataStream dsAcCode;
    dsAcCode.setData(dataAcCode);

    int mcuY[64];
    int mcuCb[64];
    int mcuCr[64];

    /* When packing RGB565 into our data we're left with some
       overflowing bits. These are contributed back into the next pixel.

       The algorithm unpacks a total of width * 8 per iteration, so
       we'll keep an extra width size for the overflowing bits which
       will pass on to the next iteration. */

    std::vector<int> r(width * 9, 0);
    std::vector<int> g(width * 9, 0);
    std::vector<int> b(width * 9, 0);

    uint8_t *buffer = rgb565Data.data();
    for (int y = 0; y < height; y += 8)
    {
        for (int x = 0; x < width; x += 8)
        {
            d_ptr->unpackMcu(ac, dc, dsAcCode, mcuY, d_ptr->y_quants);
            d_ptr->unpackMcu(ac, dc, dsAcCode, mcuCb, d_ptr->c_quants);
            d_ptr->unpackMcu(ac, dc, dsAcCode, mcuCr, d_ptr->c_quants);

            d_ptr->ycrcb_to_rgb(
                width,
                mcuY,
                mcuCb,
                mcuCr,
                r.data() + x,
                g.data() + x,
                b.data() + x);
        }

        /* Contribute overflowing bits from previous iteration. */
        for (int i = 0, ii = width * 8; i < width; i++, ii++)
        {
            r[i] += r[ii];
            g[i] += g[ii];
            b[i] += b[ii];

            r[ii] = 0;
            g[ii] = 0;
            b[ii] = 0;
        }

        d_ptr->putBlock(
            (uint16_t *)buffer,
            width,
            r.data(),
            g.data(),
            b.data());

        buffer += 8 * width * 2;
    }

    return true;
}

bool Dct::unpackBlock(
    const int blockCount,
    const std::vector<uint8_t> &imageData,
    const int quality,
    std::vector<uint8_t> &outData)
{
    outData.resize(64 * 3 * blockCount);
    d_ptr->setQuality(quality);

    return true;
}

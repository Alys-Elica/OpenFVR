#include "inversebitstream.h"

/* Private */
class InverseBitstream::InverseBitstreamPrivate {
    friend class InverseBitstream;

private:
    std::vector<uint8_t> m_data;

    size_t m_offset = 0;
    int m_bitmask = 128;
};

/* Public */
InverseBitstream::InverseBitstream()
{
    d_ptr = new InverseBitstreamPrivate;
}

InverseBitstream::~InverseBitstream()
{
    delete d_ptr;
}

void InverseBitstream::setData(const std::vector<uint8_t>& data)
{
    d_ptr->m_data = data;
    d_ptr->m_offset = 0;
    d_ptr->m_bitmask = 128;
}

int8_t InverseBitstream::next(uint8_t bits)
{
    int inverse = 1;
    int8_t value = 0;

    for (int i = 0; i < bits; i++) {
        if (d_ptr->m_offset >= d_ptr->m_data.size()) {
            break;
        }

        if (d_ptr->m_bitmask & d_ptr->m_data[d_ptr->m_offset]) {
            value = value | inverse;
        }

        d_ptr->m_bitmask >>= 1;
        inverse <<= 1;

        if (d_ptr->m_bitmask == 0) {
            d_ptr->m_bitmask = 128;
            d_ptr->m_offset += 1;
        }
    }

    return value;
}

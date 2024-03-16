#include "huffmantable.h"

namespace HuffmanTable {
void decompress(const std::vector<uint8_t>& input, std::vector<uint8_t>& output, const int uncompressedSize)
{
    output.resize(uncompressedSize);

    /* This method unpacks the frequencies from the file and then sorts them
       by their value in descending order. Each node occupies three bytes;
       the frequency, the left leaf and the right leaf. */

    /* The ac_code part begins with a null-terminated section containing the
       Huffman frequencies. The frequencies are compacted into blocks in order
       to avoid redundant zeros. Each block begins with the block starting offset,
       then continues with where the block ends. Subtract the starting offset from
       the end and we get the length - this is how many bytes follows. */

    int buffer[513 * 3 + 1] = { 0 };

    int block_start,
        block_end,
        block_len;

    size_t data_offset = 0;
    while (data_offset < 256) {
        block_start = input[data_offset++];

        /* The compacted frequencies are null-terminated.
           However, the first block commonly starts with zero as well. */
        if (data_offset > 1 && block_start == 0) {
            break;
        }

        block_end = input[data_offset++];
        block_len = block_end - block_start;

        for (int i = 0; i <= block_len; i++) {
            buffer[(block_start + i) * 3] = input[data_offset++];
        }
    }

    buffer[513 * 3] = 0x7fffffff;

    size_t left, right;
    size_t len = 256;

    buffer[len * 3] = 1;

    while (len < 513) {
        left = right = 513;

        for (size_t index = 0; index <= len; index++) {
            if (buffer[index * 3] == 0) {
                continue;
            } else if (buffer[index * 3] < buffer[left * 3]) {
                right = left;
                left = index;
            } else if (buffer[index * 3] < buffer[right * 3]) {
                right = index;
            }
        }

        if (right == 513) {
            break;
        }

        len++;

        buffer[len * 3] = buffer[left * 3] + buffer[right * 3];
        buffer[len * 3 + 1] = (int)left;
        buffer[len * 3 + 2] = (int)right;

        buffer[left * 3] = buffer[right * 3] = 0;
    }

    uint8_t bitmask = 128;

    int index;

    for (int i = 0; i < output.size(); i++) {
        index = (int)len;

        while (index > 256) {
            if (bitmask & input[data_offset]) {
                index = buffer[index * 3 + 2];
            } else {
                index = buffer[index * 3 + 1];
            }

            bitmask >>= 1;

            if (bitmask == 0) {
                data_offset++;
                bitmask = 128;
            }
        }

        if (index == 256) {
            break;
        }

        output[i] = index;
    }
}
}; // namespace HuffmanTable

#include "vrhuffman.h"

#include <vector>

/* HuffmanNode */
HuffmanNode::HuffmanNode(bool isLeaf, uint32_t dat, int freq) {
    this->isLeaf = isLeaf;
    this->data = dat;
    this->frequency = freq;

    left = nullptr;
    right = nullptr;
}

HuffmanNode::~HuffmanNode() {
    if (left) {
        delete left;
    }
    if (right) {
        delete right;
    }
}

/* VrHuffman */
VrHuffman::VrHuffman()
{
    clear();
}

VrHuffman::~VrHuffman()
{
    clear();
}

struct HuffmanCode {
    int bits;
    uint8_t length;
    uint8_t data;
};

std::vector<HuffmanCode> readHuffmanTables(uint8_t frequencies[256])
{
    int frequency[512] = { 0 };
    uint8_t flag[512];
    int up[512];
    int j;

    memset(up, -1, sizeof(up));

    for (int i = 0; i < 256; ++i) {
        frequency[i] = frequencies[i];
    }
    frequency[256] = 1;

    for (j = 257; j < 512; j++) {
        int min_freq[2] = { 256 * 256, 256 * 256 };
        int smallest[2] = { 0, 0 };
        int i;
        for (i = 0; i < j; i++) {
            if (frequency[i] == 0)
                continue;
            if (frequency[i] < min_freq[1]) {
                if (frequency[i] < min_freq[0]) {
                    min_freq[1] = min_freq[0];
                    smallest[1] = smallest[0];
                    min_freq[0] = frequency[i];
                    smallest[0] = i;
                } else {
                    min_freq[1] = frequency[i];
                    smallest[1] = i;
                }
            }
        }
        if (min_freq[1] == 256 * 256)
            break;

        frequency[j]           = min_freq[0] + min_freq[1];
        flag[smallest[0]]      = 0;
        flag[smallest[1]]      = 1;
        up[smallest[0]]        =
                up[smallest[1]]        = j;
        frequency[smallest[0]] = frequency[smallest[1]] = 0;
    }

    std::vector<HuffmanCode> codeList;
    for (j = 0; j < 257; j++) {
        int node, len = 0, bits = 0;

        for (node = j; up[node] != -1; node = up[node]) {
            bits += flag[node] << len;
            len++;
            if (len > 31) {
                // can this happen at all ?
                // length overflow
            }
        }

        if (len > 0) {
            HuffmanCode code;
            code.bits = bits;
            code.length = len;
            code.data = j;

            codeList.push_back(code);
        }
    }

    return codeList;
}

void VrHuffman::buildTree(uint8_t frequencies[256])
{
    clear();

    std::vector<HuffmanCode> codeList = readHuffmanTables(frequencies);

    if (codeList.empty()) {
        return;
    }

    m_rootNode = new HuffmanNode(false, -1, -1);

    for (const HuffmanCode &code : codeList) {
        HuffmanNode *currentNode = m_rootNode;

        int mask = (1 << (code.length - 1));
        for (int i = 0; i < code.length; ++i) {
            bool isSet = ((code.bits & mask) > 0);

            if (mask > 0) {
                // There is still bits to use
                if (isSet) {
                    if (currentNode->right == nullptr) {
                        currentNode->right = new HuffmanNode(false, -1, -1);
                    }
                    currentNode = currentNode->right;
                } else {
                    if (currentNode->left == nullptr) {
                        currentNode->left = new HuffmanNode(false, -1, -1);
                    }
                    currentNode = currentNode->left;
                }
            } else {
                // We reached the end of the code
            }

            mask >>= 1;
        }
        currentNode->isLeaf = true;
        currentNode->data = code.data;
    }
}

int VrHuffman::uncompress(const int compressedSize, const int uncompressedSize, const uint8_t *compressedData, uint8_t *uncompressedData)
{
    init();

    /* HEADER */
    uint8_t normFrequenciesData[256];
    for (int i = 0; i < 256; ++i) {
        normFrequenciesData[i] = 0;
    }

    // Load normalized frequencies data
    uint8_t offsetBegin = compressedData[m_bitIndex++];

    do {
        uint8_t offsetEnd = compressedData[m_bitIndex++];

        if (offsetBegin <= offsetEnd) {
            int size = (offsetEnd - offsetBegin) + 1;

            for (int i = 0; i < size; ++i) {
                uint8_t byte = compressedData[m_bitIndex++];
                normFrequenciesData[(offsetBegin) + i] = byte;
            }
        }

        offsetBegin = compressedData[m_bitIndex++];
    } while (offsetBegin != 0);

    buildTree(normFrequenciesData);

    /* DATA UNCOMPRESSION */
    int finalSize = -1;

    if (m_rootNode != nullptr) {
        finalSize = 0;

        while (finalSize < uncompressedSize && m_bitIndex < compressedSize) {
            HuffmanNode *curNode = m_rootNode;
            while (!curNode->isLeaf) {
                uint8_t bit = readBit(compressedData);

                if (bit == 0) {
                    if (curNode->left == nullptr) {
                        break;
                    }
                    curNode = curNode->left;
                } else {
                    if (curNode->right == nullptr) {
                        break;
                    }
                    curNode = curNode->right;
                }
            }

            if (!curNode->isLeaf || curNode->data == 256) {
                break;
            } else {
                uncompressedData[finalSize++] = curNode->data;
            }
        }
    }

    return finalSize;
}

void VrHuffman::clear()
{
    if (m_rootNode) {
        delete m_rootNode;
    }

    m_rootNode = nullptr;
}

void VrHuffman::init()
{
    m_bitMask = 0x80;
    m_bitIndex = 0;
}

uint8_t VrHuffman::readBit(const uint8_t *data)
{
    uint8_t byte = data[m_bitIndex];
    uint8_t tmpMask = m_bitMask;

    m_bitMask >>= 1;

    if (m_bitMask == 0) {
        m_bitMask = 0x80;
        ++m_bitIndex;
    }

    return byte & tmpMask;
}

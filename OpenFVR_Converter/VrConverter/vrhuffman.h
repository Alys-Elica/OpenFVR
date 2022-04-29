#ifndef VRHUFFMAN_H
#define VRHUFFMAN_H

#include <cstdint>

struct HuffmanNode {
    HuffmanNode *left;
    HuffmanNode *right;

    bool isLeaf;
    uint32_t data;
    int frequency;

    HuffmanNode(bool isLeaf, uint32_t dat, int freq);

    ~HuffmanNode();
};

class VrHuffman
{
public:
    VrHuffman();
    ~VrHuffman();

    void buildTree(uint8_t frequencies[256]);
    int uncompress(const int compressedSize, const int uncompressedSize, const uint8_t *compressedData, uint8_t *uncompressedData);

private:
    void clear();

    void init();
    uint8_t readBit(const uint8_t *data);

    HuffmanNode *m_rootNode = nullptr;

    uint8_t m_bitMask;
    int m_bitIndex;
};

#endif // VRHUFFMAN_H

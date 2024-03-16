#ifndef HUFFMANTABLE_H
#define HUFFMANTABLE_H

#include "libfvr_globals.h"

#include <cstdint>
#include <vector>

namespace HuffmanTable {
LIBFVR_EXPORT void decompress(const std::vector<uint8_t>& input, std::vector<uint8_t>& output, const int uncompressedSize);
}; // namespace HuffmanTable

#endif // HUFFMANTABLE_H

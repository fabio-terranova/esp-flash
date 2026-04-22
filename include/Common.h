#ifndef __COMMON_H__
#define __COMMON_H__

#include <cstdint>
#include <vector>

using Bytes = std::vector<uint8_t>;

void printHex(const Bytes& data);

#endif // __COMMON_H__

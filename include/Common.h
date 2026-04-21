#ifndef INCLUDE_SRC_COMMON_H_
#define INCLUDE_SRC_COMMON_H_

#include <cstdint>
#include <vector>

using Bytes = std::vector<uint8_t>;

void printHex(const Bytes& data);

#endif // INCLUDE_SRC_COMMON_H_

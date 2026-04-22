#ifndef __COMMON_H__
#define __COMMON_H__

#include <cstdint>
#include <termios.h>
#include <vector>

using Bytes = std::vector<uint8_t>;

void printHex(const Bytes& data);

namespace Config {
constexpr unsigned int BAUD = B115200;
}

#endif // __COMMON_H__

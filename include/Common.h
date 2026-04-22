#ifndef __COMMON_H__
#define __COMMON_H__

#include <cstdint>
#include <termios.h>
#include <vector>

using Bytes = std::vector<uint8_t>;

void printHex(const Bytes& data);
void msSleep(unsigned int ms);

namespace Config {
constexpr unsigned int kBaud            = B115200;
constexpr unsigned int kMaxSyncAttempts = 10;
} // namespace Config

#endif // __COMMON_H__

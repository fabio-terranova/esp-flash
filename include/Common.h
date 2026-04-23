#ifndef __COMMON_H__
#define __COMMON_H__

#include <cstdint>
#include <format>
#include <string>
#include <termios.h>
#include <vector>

using Bytes = std::vector<uint8_t>;

template <typename T>
std::string toHexString(T val) {
  int width = sizeof(val) * 2;

  return std::format("{:0{}x}", +val, width);
}

void printBuffer(const std::vector<uint8_t>& buffer, uint32_t stride,
                 bool printAscii = false);
void msSleep(unsigned int ms);

namespace Config {
constexpr unsigned int kBaud            = B115200;
constexpr unsigned int kMaxSyncAttempts = 10;
} // namespace Config

#endif // __COMMON_H__

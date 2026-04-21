#include "Common.h"
#include <iomanip>
#include <iostream>

void printHex(const Bytes& data) {
  for (const auto byte : data) {
    std::cout << std::hex << std::setw(2) << std::setfill('0')
              << static_cast<int>(byte) << ' ';
  }
  std::cout << std::dec;
}
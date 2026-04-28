#include "Common.h"
#include <chrono>
#include <cstdint>
#include <iostream>
#include <thread>

void msSleep(unsigned int ms) {
  std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void printBuffer(const std::vector<uint8_t>& buffer, uint32_t stride,
                 bool printAscii) {
  for (uint32_t addr{0}; addr < buffer.size(); addr += stride) {
    std::cout << toHexString(addr) << ": ";

    for (uint32_t i = 0; i < stride; ++i) {
      if (addr + i < buffer.size()) {
        std::cout << toHexString(buffer[addr + i]) << " ";
      } else {
        std::cout << "   ";
      }
    }

    if (printAscii) {
      std::cout << "| ";

      for (uint32_t i = 0; i < stride; ++i) {
        if (addr + i < buffer.size()) {
          uint8_t ch = buffer[addr + i];
          std::cout << (std::isprint(ch) ? static_cast<char>(ch) : '.');
        } else {
          std::cout << " ";
        }
      }
    }

    std::cout << '\n';
  }
}

uint8_t checksum(const Bytes& data) {
  uint8_t checksum{0xEF};
  for (const auto byte : data) {
    checksum ^= byte;
  }
  return checksum;
}

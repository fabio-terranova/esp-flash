#include "Common.h"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <thread>

void printHex(const Bytes& data) {
  for (const auto byte : data) {
    std::cout << std::hex << std::setw(2) << std::setfill('0')
              << static_cast<int>(byte) << ' ';
  }
  std::cout << std::dec;
}

void msSleep(unsigned int ms) {
  std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}
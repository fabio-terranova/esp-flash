#include "ESP32.h"
#include <exception>
#include <iostream>
#include <string>

int main(int argc, const char** argv) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <serial_port>\n";
    return 1;
  }

  try {
    ESP32::Device esp32{argv[1]};
    esp32.reset_into_bootloader();
    esp32.sync();
  } catch (const std::exception& e) {
    std::cerr << e.what() << '\n';
    return 1;
  }

  return 0;
}

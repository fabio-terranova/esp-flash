#include "Common.h"
#include "ESP32.h"
#include <exception>
#include <iostream>

int main(int argc, const char* argv[]) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <serial_port>\n";
    return 1;
  }

  try {
    std::unique_ptr<Serial::IPort> port =
        std::make_unique<Serial::Port>(argv[1]);

    ESP32::Device esp32{std::move(port)};

    esp32.resetIntoBootloader();
    esp32.sync(Config::kMaxSyncAttempts);

    if (esp32.checkChip())
      std::cout << "Found an ESP32!\n";
    else
      std::cout << "Wrong chip id...\n";

  } catch (const std::exception& e) {
    std::cerr << e.what() << '\n';
    return 1;
  }

  return 0;
}

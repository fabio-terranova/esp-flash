#ifndef __ESP32_H__
#define __ESP32_H__

#include "Common.h"
#include "Serial.h"
#include <cstdint>
#include <memory>

namespace ESP32 {

enum class Direction : uint8_t {
  hostToDevice = 0x00,
  deviceToHost = 0x01,
};

enum class Command : uint8_t {
  SYNC = 0x08,
};

struct __attribute__((packed)) CommandHeader {
  Direction direction{Direction::hostToDevice}; // uint8_t
  Command   command;                            // uint8_t
  uint16_t  size;
  uint32_t  checksum;
};

struct __attribute__((packed)) ResponseHeader {
  Direction direction{Direction::deviceToHost}; // uint8_t
  Command   command;                            // uint8_t
  uint16_t  size;
  uint32_t  value;
};

class Device {
public:
  explicit Device(std::unique_ptr<Serial::IPort> port)
      : m_port(std::move(port)) {
    m_port->configure(Config::kBaud);
  }

  void resetIntoBootloader();
  void sync();

  size_t write(const Bytes& packet);
  size_t read(Bytes& buffer);

  Serial::IPort& port() { return *m_port; }

private:
  std::unique_ptr<Serial::IPort> m_port;

  Bytes syncPacket();
};
} // namespace ESP32

#endif // __ESP32_H__

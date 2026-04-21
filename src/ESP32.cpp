#include "ESP32.h"
#include "Common.h"
#include "Serial.h"
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <sys/types.h>
#include <system_error>
#include <thread>
#include <unistd.h>

namespace ESP32 {
constexpr uint8_t SYNC = 0x08;

uint8_t checksum(const Bytes& data) {
  uint8_t checksum{0xEF};
  for (const auto byte : data) {
    checksum ^= byte;
  }
  return checksum;
}

Bytes commandPacket(uint8_t cmd, const Bytes& data) {
  CommandHeader header{.command  = cmd,
                       .size     = static_cast<uint16_t>(data.size()),
                       .checksum = static_cast<uint32_t>(checksum(data))};

  Bytes packet(sizeof(header) + data.size());
  std::memcpy(packet.data(), &header, sizeof(header));
  std::memcpy(packet.data() + sizeof(header), data.data(), data.size());

  return packet;
}

void Device::reset_into_bootloader() {
  // DTR: GPIO0
  // RTS: EN
  using Serial::HIGH;
  using Serial::LOW;

  m_port.set_DTR(HIGH);
  std::this_thread::sleep_for(std::chrono::milliseconds(50));

  m_port.set_DTR(LOW);
  m_port.set_RTS(HIGH);
  std::this_thread::sleep_for(std::chrono::milliseconds(50));

  m_port.set_DTR(HIGH);
  m_port.set_RTS(LOW);
  std::this_thread::sleep_for(std::chrono::milliseconds(50));

  m_port.set_DTR(LOW);
}

void Device::sync() {
  Bytes data{0x07, 0x07, 0x12, 0x20};
  data.insert(data.end(), 32, 0x55);

  Bytes packet{Serial::SLIP::encode(commandPacket(SYNC, data))};
  write(packet);

  // TODO: Wait for and check response to SYNC command
}

size_t Device::write(const Bytes& packet) {
  ssize_t written = ::write(m_port.fd(), packet.data(), packet.size());
  if (written < 0) {
    throw std::system_error(errno, std::system_category(), "Write failed");
  }
  return static_cast<size_t>(written);
}

size_t Device::read(Bytes& buffer) {
  ssize_t read = ::read(m_port.fd(), buffer.data(), buffer.size());
  if (read < 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      return 0;
    }
    throw std::system_error(errno, std::system_category(), "Read failed");
  }

  return static_cast<size_t>(read);
}
} // namespace ESP32

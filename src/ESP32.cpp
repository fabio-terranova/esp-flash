#include "ESP32.h"
#include "Common.h"
#include "Serial.h"
#include <cerrno>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <sys/types.h>
#include <system_error>
#include <thread>
#include <unistd.h>

namespace ESP32 {
constexpr int kMaxSyncAttempts = 10;

uint8_t checksum(const Bytes& data) {
  uint8_t checksum{0xEF};
  for (const auto byte : data) {
    checksum ^= byte;
  }
  return checksum;
}

Bytes commandPacket(Command cmd, const Bytes& data) {
  CommandHeader header{.command  = cmd,
                       .size     = static_cast<uint16_t>(data.size()),
                       .checksum = static_cast<uint32_t>(checksum(data))};

  Bytes packet(sizeof(header) + data.size());
  std::memcpy(packet.data(), &header, sizeof(header));
  std::memcpy(packet.data() + sizeof(header), data.data(), data.size());

  return packet;
}

void Device::resetIntoBootloader() {
  // DTR: GPIO0
  // RTS: EN
  using Serial::HIGH;
  using Serial::LOW;

  m_port->setDTR(HIGH);
  std::this_thread::sleep_for(std::chrono::milliseconds(50));

  m_port->setDTR(LOW);
  m_port->setRTS(HIGH);
  std::this_thread::sleep_for(std::chrono::milliseconds(50));

  m_port->setDTR(HIGH);
  m_port->setRTS(LOW);
  std::this_thread::sleep_for(std::chrono::milliseconds(50));

  m_port->setDTR(LOW);
}

Bytes Device::syncPacket() {
  static Bytes packet = [] {
    Bytes data{0x07, 0x07, 0x12, 0x20};
    data.insert(data.end(), 32, 0x55);
    return Serial::SLIP::encode(commandPacket(Command::SYNC, data));
  }();

  return packet;
}

void Device::sync() {
  for (int attempt{}; attempt < kMaxSyncAttempts; ++attempt) {
    write(syncPacket());
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    Serial::SLIP::Decoder decoder;
    Bytes                 buffer(256);

    size_t bytesRead = read(buffer);
    for (size_t i{0}; i < bytesRead; ++i) {
      auto frame = decoder.feed(buffer[i]);
      if (!frame.has_value())
        continue;

      const Bytes& response = frame.value();

      ResponseHeader header;
      std::memcpy(&header, response.data(), sizeof(ResponseHeader));

      if (header.direction != Direction::deviceToHost ||
          header.command != Command::SYNC)
        continue;

      return;
    }
  }

  throw std::runtime_error("SYNC failed: no valid response from device");
}

Bytes Device::readRegPacket(uint32_t address) {
  Bytes data(4);
  std::memcpy(data.data(), &address, sizeof(address));
  return Serial::SLIP::encode(commandPacket(Command::READ_REG, data));
}

bool Device::checkChip() {
  write(readRegPacket(0x40001000));
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  Serial::SLIP::Decoder decoder;
  Bytes                 buffer(256);
  size_t                bytesRead = read(buffer);
  for (size_t i{0}; i < bytesRead; ++i) {
    auto frame = decoder.feed(buffer[i]);
    if (!frame.has_value())
      continue;

    const Bytes&   response = frame.value();
    ResponseHeader header;
    std::memcpy(&header, response.data(), sizeof(ResponseHeader));
    if (header.direction != Direction::deviceToHost ||
        header.command != Command::READ_REG)
      continue;

    return header.value == MAGIC_VALUE;
  }

  throw std::runtime_error("Chip check failed: no valid response from device");
}

size_t Device::write(const Bytes& packet) {
  ssize_t written = ::write(m_port->fd(), packet.data(), packet.size());
  if (written < 0) {
    throw std::system_error(errno, std::system_category(), "Write failed");
  }
  return static_cast<size_t>(written);
}

size_t Device::read(Bytes& buffer) {
  ssize_t read = ::read(m_port->fd(), buffer.data(), buffer.size());
  if (read < 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      return 0;
    }
    throw std::system_error(errno, std::system_category(), "Read failed");
  }

  return static_cast<size_t>(read);
}
} // namespace ESP32

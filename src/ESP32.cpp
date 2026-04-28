#include "ESP32.h"
#include "Common.h"
#include "Serial.h"
#include <cerrno>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <optional>
#include <stdexcept>
#include <sys/types.h>
#include <system_error>
#include <thread>
#include <unistd.h>

namespace ESP32 {
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
}

Bytes Request::rawPacket() const {
  CommandHeader header{.command  = m_cmd,
                       .size     = static_cast<uint16_t>(m_payload.size()),
                       .checksum = static_cast<uint32_t>(checksum(m_payload))};

  Bytes packet(sizeof(header) + m_payload.size());
  std::memcpy(packet.data(), &header, sizeof(header));
  std::memcpy(packet.data() + sizeof(header), m_payload.data(),
              m_payload.size());

  return packet;
}
std::optional<Response> Device::transact(const Request& req,
                                         unsigned int   maxAttempts) {
  const Bytes   encodedPacket = req.encodedPacket();
  const Command expectedCmd   = req.command();

  for (unsigned int attempt{}; attempt < maxAttempts; ++attempt) {
    write(encodedPacket);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    Serial::SLIP::Decoder decoder;
    Bytes                 buffer(256);
    size_t                bytesRead = read(buffer);

    for (size_t i{0}; i < bytesRead; ++i) {
      auto frame = decoder.feed(buffer[i]);
      if (!frame.has_value()) {
        continue;
      }

      const Bytes& rawResponse = frame.value();

      if (rawResponse.size() < sizeof(ResponseHeader)) {
        continue;
      }

      Response response(rawResponse);

      if (response.direction() == Direction::deviceToHost &&
          response.command() == expectedCmd) {
        return response;
      }
    }
  }

  return std::nullopt;
}

Bytes Device::syncPayload() {
  static Bytes payload = [] {
    Bytes data{0x07, 0x07, 0x12, 0x20};
    data.insert(data.end(), 32, 0x55);
    return data;
  }();

  return payload;
}

void Device::sync(const unsigned int maxAttempts) {
  static Request req(Command::SYNC, syncPayload());
  auto           response = transact(req, maxAttempts);

  if (!response.has_value()) {
    throw std::runtime_error("Sync failed: no valid response from device");
  }
}

Bytes Device::readRegPayload(uint32_t address) {
  Bytes data(4);
  std::memcpy(data.data(), &address, sizeof(address));
  return data;
}

bool Device::checkChip() {
  static Request req(Command::READ_REG, readRegPayload(0x40001000));

  auto response = transact(req);

  if (!response.has_value()) {
    throw std::runtime_error(
        "Chip check failed: no valid response from device");
  }

  return response.value().value() == MAGIC_VALUE;
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

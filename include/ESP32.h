#ifndef __ESP32_H__
#define __ESP32_H__

#include "Common.h"
#include "Serial.h"
#include <cstdint>
#include <cstring>
#include <memory>

namespace ESP32 {
constexpr uint32_t MAGIC_VALUE = 0x00F01D83;

enum class Direction : uint8_t {
  hostToDevice = 0x00,
  deviceToHost = 0x01,
};

enum class Command : uint8_t {
  SYNC       = 0x08,
  READ_REG   = 0x0a,
  SPI_ATTACH = 0x0d,
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

class Request {
public:
  Request(const Command cmd, const Bytes& payload)
      : m_cmd(cmd), m_payload(payload) {}

  Command command() const { return m_cmd; }
  Bytes   encodedPacket() const { return Serial::SLIP::encode(rawPacket()); }

private:
  Command m_cmd;
  Bytes   m_payload;

  Bytes rawPacket() const;
};

class Response {
public:
  explicit Response(const Bytes& rawData) : m_raw(rawData) {
    std::memcpy(&m_header, m_raw.data(), sizeof(ResponseHeader));
  }

  Direction direction() const { return m_header.direction; }
  Command   command() const { return m_header.command; }
  uint32_t  value() const { return m_header.value; }

  const Bytes& raw() const { return m_raw; }

private:
  Bytes          m_raw;
  ResponseHeader m_header;
};

class Device {
public:
  explicit Device(std::unique_ptr<Serial::IPort> port)
      : m_port(std::move(port)) {
    m_port->configure(Config::kBaud);
  }

  void resetIntoBootloader();
  void sync(const unsigned int maxAttempts);
  bool checkChip();

  size_t write(const Bytes& packet);
  size_t read(Bytes& buffer);

  std::optional<Response> transact(const Request& req,
                                   unsigned int   maxAttempts = 1);

  Serial::IPort& port() { return *m_port; }

private:
  std::unique_ptr<Serial::IPort> m_port;

  Bytes syncPayload();
  Bytes readRegPayload(uint32_t address);
};
} // namespace ESP32

#endif // __ESP32_H__

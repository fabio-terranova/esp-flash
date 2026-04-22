#ifndef __SERIAL_H__
#define __SERIAL_H__

#include "Common.h"
#include <cstdint>
#include <fcntl.h>
#include <optional>
#include <sys/ioctl.h>
#include <system_error>
#include <unistd.h>
#include <utility>

namespace Serial {
constexpr bool HIGH = true;
constexpr bool LOW  = false;

// SLIP special character codes
namespace SLIP {
constexpr uint8_t END     = 0300; // End of packet
constexpr uint8_t ESC     = 0333; // Byte stuffing
constexpr uint8_t ESC_END = 0334; // ESC ESC_END means END data byte
constexpr uint8_t ESC_ESC = 0335; // ESC ESC_END means END data byte

Bytes encode(const Bytes& data);

class Decoder {
public:
  std::optional<Bytes> feed(uint8_t byte);
  void                 reset();

  const Bytes& buffer() const { return m_buffer; }

private:
  enum State { normal, escape } m_state = normal;
  Bytes m_buffer;
};
} // namespace SLIP

class IPort {
public:
  virtual ~IPort() = default;

  virtual void configure(unsigned int speed) = 0;
  virtual void setDTR(bool state)            = 0;
  virtual void setRTS(bool state)            = 0;
  virtual int  fd() const noexcept           = 0;
};

class Port : public IPort {
public:
  explicit Port(const std::string& file)
      : m_fd(::open(file.c_str(), O_RDWR | O_NOCTTY | O_SYNC)) {
    if (m_fd < 0) {
      throw std::system_error(errno, std::system_category(),
                              "Failed to open serial port: " + file);
    }
  }

  ~Port() {
    if (m_fd > 0)
      ::close(m_fd);
  }

  Port(const Port&)            = delete;
  Port& operator=(const Port&) = delete;

  Port(Port&& other) noexcept : m_fd(std::exchange(other.m_fd, -1)) {}

  Port& operator=(Port&& other) {
    if (this != &other) {
      if (m_fd >= 0)
        ::close(m_fd);
      m_fd = std::exchange(other.m_fd, -1);
    }

    return *this;
  }

  void configure(unsigned int speed);

  void setDTR(bool state);
  void setRTS(bool state);

  int fd() const noexcept { return m_fd; }

private:
  int m_fd{-1};
};

} // namespace Serial

#endif // __SERIAL_H__

#ifndef INCLUDE_SRC_SLIP_H_
#define INCLUDE_SRC_SLIP_H_

#include "Common.h"
#include <cstdint>
#include <fcntl.h>
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
} // namespace SLIP

class Port {
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

  void set_DTR(bool state);
  void set_RTS(bool state);

  int fd() const noexcept { return m_fd; }

private:
  int m_fd{-1};
};

} // namespace Serial

#endif // INCLUDE_SRC_SLIP_H_

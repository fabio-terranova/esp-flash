#include "Serial.h"
#include <cstdint>
#include <optional>
#include <termios.h>

namespace Serial {
namespace SLIP {
Bytes encode(const Bytes& data) {
  Bytes packet;
  packet.reserve(2 * data.size() + 2); // Optimization for worst case scenario

  packet.push_back(SLIP::END);
  for (const auto byte : data) {
    if (byte == SLIP::END) {
      packet.push_back(SLIP::ESC);
      packet.push_back(SLIP::ESC_END);
    } else if (byte == SLIP::ESC) {
      packet.push_back(SLIP::ESC);
      packet.push_back(SLIP::ESC_ESC);
    } else {
      packet.push_back(byte);
    }
  }
  packet.push_back(SLIP::END);

  return packet;
}

std::optional<Bytes> Decoder::feed(uint8_t byte) {
  if (byte == END) {
    if (m_buffer.empty()) {
      // Ignore empty frames
      return std::nullopt;
    }
    if (m_state == State::escape) {
      // END after ESC is invalid, reset state and buffer
      m_state = State::normal;
      m_buffer.clear();
      return std::nullopt;
    }
    // Return completed frame
    Bytes frame;
    frame.swap(m_buffer);
    return frame;
  }
  // Handle ESC byte
  if (byte == ESC) {
    m_state = State::escape;
    return std::nullopt;
  }
  // Handle bytes after ESC
  if (m_state == State::escape) {
    switch (byte) {
    case ESC_END:
      m_buffer.push_back(END);
      break;
    case ESC_ESC:
      m_buffer.push_back(ESC);
      break;
    default:
      m_buffer.clear();
      break;
    }
    m_state = State::normal;
    return std::nullopt;
  }

  // Normal byte
  m_buffer.push_back(byte);
  return std::nullopt;
}

void Decoder::reset() {
  m_buffer.clear();
  m_state = State::normal;
}
} // namespace SLIP

void Port::setDTR(bool state) {
  int ctrlBit = TIOCM_DTR;
  if (ioctl(m_fd, state ? TIOCMBIS : TIOCMBIC, &ctrlBit) == -1) {
    throw std::system_error(errno, std::system_category(), "Failed to set DTR");
  }
}

void Port::setRTS(bool state) {
  int ctrlBit = TIOCM_RTS;
  if (ioctl(m_fd, state ? TIOCMBIS : TIOCMBIC, &ctrlBit) == -1) {
    throw std::system_error(errno, std::system_category(), "Failed to set RTS");
  }
}

void Port::configure(unsigned int speed) {
  termios settings{};
  if (tcgetattr(m_fd, &settings) != 0)
    throw std::system_error(errno, std::system_category(),
                            "Failed to set termios attributes");

  cfmakeraw(&settings);
  cfsetspeed(&settings, speed);

  settings.c_cc[VTIME] = 1; // Wait for up to 1 decisecond for data
  settings.c_cc[VMIN]  = 0; // Return immediately with what is available

  if (tcsetattr(m_fd, TCSANOW, &settings) != 0)
    throw std::system_error(errno, std::system_category(),
                            "Failed to set termios attributes");
}
} // namespace Serial

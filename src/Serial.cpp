#include "Serial.h"

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
} // namespace SLIP

void Port::set_DTR(bool state) {
  int control_bit = TIOCM_DTR;
  if (ioctl(m_fd, state ? TIOCMBIS : TIOCMBIC, &control_bit) == -1) {
    throw std::system_error(errno, std::system_category(), "Failed to set DTR");
  }
}

void Port::set_RTS(bool state) {
  int control_bit = TIOCM_RTS;
  if (ioctl(m_fd, state ? TIOCMBIS : TIOCMBIC, &control_bit) == -1) {
    throw std::system_error(errno, std::system_category(), "Failed to set RTS");
  }
}
} // namespace Serial

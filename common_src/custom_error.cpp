#include <string>

#include "custom_error.h"

SocketClosed::SocketClosed(const std::string &msg_rcv) : msg(msg_rcv) {}

const char *SocketClosed::what() const noexcept { return msg.c_str(); }

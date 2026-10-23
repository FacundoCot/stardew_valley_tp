#ifndef CUSTOM_ERROR_H
#define CUSTOM_ERROR_H

#include <stdexcept>
#include <string>

class SocketClosed : std::exception {
private:
  std::string msg;

public:
  explicit SocketClosed(const std::string &msg);

  const char *what() const noexcept override;
};

#endif

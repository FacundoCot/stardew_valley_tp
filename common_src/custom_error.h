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

class AssetNotFound : std::exception {
private:
  std::string msg;
  
public:
  explicit AssetNotFound(const std::string &msg);
  const char *what() const noexcept override;
};

class ItemNotFound : std::exception {
private:
  std::string msg;

public:
  explicit ItemNotFound(const std::string &msg);
  const char *what() const noexcept override;


};

#endif

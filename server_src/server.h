#ifndef SERVER_H
#define SERVER_H

#include "../common_src/queue.h"
#include "../common_src/socket.h"

#include "acceptor.h"
#include <memory>

class Server {
private:
  Queue<std::shared_ptr<ClientAction>> actions;
  Acceptor acceptor;

public:
  explicit Server(Socket &&skt);
  void run();
};

#endif

#ifndef CLIENT_H
#define CLIENT_H

#include "client_protocol.h"
#include "eventhandler.h"

class Client {

private:
  EventHandler e_handler;
  ClientProtocol protocol;

public:
  Client(EventHandler &&e_handler_rcv, ClientProtocol &&protocol);

  void run();
};

#endif

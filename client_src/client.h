#ifndef CLIENT_H
#define CLIENT_H

#include "../common_src/messages.h"
#include "../common_src/queue.h"
#include "client_protocol.h"
#include "eventhandler.h"
#include "receiver.h"
#include "sender.h"

class Client {

private:
  EventHandler e_handler;

  Queue<ServerMessage> responses;
  Queue<ClientMessage> commands;

  ClientProtocol protocol;
  Receiver receiver;
  Sender sender;

  bool running = true;

public:
  Client(EventHandler &&e_handler_rcv, ClientProtocol &&protocol);

  void run();
  void stop();

  ~Client();
};

#endif

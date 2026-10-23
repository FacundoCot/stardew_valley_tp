#ifndef RECEIVER_H
#define RECEIVER_H

#include "../common_src/messages.h"
#include "../common_src/queue.h"
#include "../common_src/socket.h"
#include "../common_src/thread.h"
#include "client_protocol.h"

class Receiver : public Thread {
private:
  Queue<ServerMessage> &responses;
  ClientProtocol &protocol;

public:
  Receiver(Queue<ServerMessage> &responses, ClientProtocol &protocol);

  void run() override;
};

#endif

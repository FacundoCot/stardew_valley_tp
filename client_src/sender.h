#ifndef SENDER_H
#define SENDER_H

#include "../common_src/messages.h"
#include "../common_src/queue.h"
#include "../common_src/thread.h"
#include "client_protocol.h"

class Sender : public Thread {
private:
  ClientProtocol &protocol;
  Queue<ClientMessage> &outgoing;

public:
  Sender(ClientProtocol &protocol_rcv, Queue<ClientMessage> &outgoing_rcv);

  void run() override;
};

#endif

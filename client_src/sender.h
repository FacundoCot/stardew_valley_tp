#ifndef SENDER_H
#define SENDER_H

#include "../common_src/messages.h"
#include "../common_src/queue.h"
#include "../common_src/thread.h"
#include "client_protocol.h"

class Sender : public Thread {
private:
  Queue<ClientMessage> &outgoing;
  ClientProtocol &protocol;

public:
  Sender(Queue<ClientMessage> &outgoing_rcv, ClientProtocol &protocol_rcv);

  void run() override;
};

#endif

#include "sender.h"
#include "../common_src/custom_error.h"
#include "../common_src/messages.h"
#include "../common_src/queue.h"
#include "client_protocol.h"

Sender::Sender(ClientProtocol &protocol_rcv, Queue<ClientMessage> &outgoing_rcv)
    : protocol(protocol_rcv), outgoing(outgoing_rcv) {}

void Sender::run() {
  try {
    while (should_keep_running()) {
      ClientMessage msg = outgoing.pop();

      protocol.send_to_server(msg);
    }
  } catch (const SocketClosed &) {
    outgoing.close();
  }
}

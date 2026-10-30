#include <exception>

#include "../common_src/custom_error.h"
#include "../common_src/messages.h"
#include "../common_src/queue.h"
#include "../common_src/socket.h"
#include "client_protocol.h"
#include "receiver.h"

Receiver::Receiver(Queue<ServerMessage> &responses_rcv,
                   ClientProtocol &protocol_rcv)
    : responses(responses_rcv), protocol(protocol_rcv) {}

void Receiver::run() {
  try {
    while (should_keep_running()) {
      ServerMessage response = protocol.rcv_from_server();

      responses.push(response);
    }

  } catch (const SocketClosed &e) {
    responses.close();
  }
}

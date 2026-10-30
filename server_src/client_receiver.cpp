#include "client_receiver.h"

#include <exception>
#include <memory>

#include "server_protocol.h"

ClientReceiver::ClientReceiver(Socket &skt,
                               Queue<std::shared_ptr<ClientAction>> &actions,
                               int client_id)
    : skt(skt), actions(actions), client_id(client_id) {}

void ClientReceiver::run() {
  ServerProtocol protocol(skt);

  try {
    while (should_keep_running()) {
      std::shared_ptr<ClientAction> action = protocol.recv_action();

      action->set_client_id(client_id);

      actions.push(action);
    }
  } catch (const ClosedQueue &) {
    return;
  } catch (const std::exception &) {
    // Desconexion o error de comunicacion.
  }

  if (should_keep_running()) {
    auto disconnect = std::make_shared<DisconnectAction>();
    disconnect->set_client_id(client_id);

    try {
      actions.push(disconnect);
    } catch (const ClosedQueue &) {
      // El servidor esta cerrando la cola.
    }
  }
}

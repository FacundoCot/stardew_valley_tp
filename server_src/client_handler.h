#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include <memory>

#include "../common_src/socket.h"
#include "../common_src/messages.h"
#include "../common_src/queue.h"
#include "client_receiver.h"
#include "client_sender.h"
#include "client_action.h"

class ClientHandler {
private:
    Socket skt;
    Queue<ServerMessage> outgoing;

    ClientReceiver receiver;
    ClientSender sender;

    bool stopped = false;

public:
    explicit ClientHandler(Socket&& skt,
                  Queue<std::shared_ptr<ClientAction>>& actions,
                  int client_id);

    void start();
    void stop();

    bool is_finished();

    ~ClientHandler();
};

#endif

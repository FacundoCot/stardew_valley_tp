#ifndef CLIENT_RECEIVER_H
#define CLIENT_RECEIVER_H

#include <memory>

#include "../common_src/thread.h"
#include "../common_src/socket.h"
#include "../common_src/queue.h"

#include "client_action.h"

class ClientReceiver : public Thread {
private:
    Socket& skt;
    Queue<std::shared_ptr<ClientAction>>& actions;
    int client_id;

public:
    ClientReceiver(Socket& skt,
                   Queue<std::shared_ptr<ClientAction>>& actions,
                   int client_id);

    void run() override;
};

#endif
#ifndef CLIENT_SENDER_H
#define CLIENT_SENDER_H

#include "../common_src/thread.h"
#include "../common_src/socket.h"
#include "../common_src/queue.h"
#include "server_protocol.h"

struct ServerMessage;

class ClientSender : public Thread {
private:
    Socket& skt;
    ServerProtocol protocol;
    //una por cada clienthandler, cada clienthandler tiene su propia cola de mensajes salientes
    //la llena el gameloop, aca solo pop y manda por socket
    Queue<ServerMessage>& outgoing;

public:
    explicit ClientSender(Socket& skt,
                 Queue<ServerMessage>& outgoing);

    void run() override;
};

#endif
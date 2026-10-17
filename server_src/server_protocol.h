#ifndef SERVER_PROTOCOL_H
#define SERVER_PROTOCOL_H

#include <memory>
#include <string>

#include "../common_src/messages.h"
#include "../common_src/socket.h"

#include "client_action.h"

class ServerProtocol {
private:
    Socket& skt;

    void recv_or_throw(void* data, unsigned int size);
    void send_or_throw(const void* data, unsigned int size);

    std::string recv_string();
    void send_string(const std::string& str);

public:
    explicit ServerProtocol(Socket& skt);

    // se usa shared pointer porque la queue hace copias de los punteros
    // entonces no podemos usar unique_ptr, y no queremos usar raw pointer porque
    // no queremos preocuparnos por liberar la memoria de los punteros que
    // se guardan en la queue, entonces usamos shared_ptr para que se libere
    // automáticamente cuando no haya más referencias a ese puntero
    std::shared_ptr<ClientAction> recv_action();
    void send_message(const ServerMessage& message);
};

#endif
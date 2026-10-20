#ifndef CLIENT_REGISTRY_H
#define CLIENT_REGISTRY_H

#include <map>
#include <mutex>

#include "../common_src/messages.h"
#include "../common_src/queue.h"

class ClientRegistry {
private:
    std::mutex mtx;
    std::map<int, Queue<ServerMessage>*> queues;

public:
    // Anota la cola de salida de un cliente nuevo.
    void add(int client_id, Queue<ServerMessage>* outgoing);

    // Quita al cliente. Debe llamarse antes de destruir su handler.
    void remove(int client_id);

    // Pone el mensaje en la cola del cliente. Si el cliente ya no
    // existe, no hace nada.
    void send(int client_id, const ServerMessage& message);
};

#endif
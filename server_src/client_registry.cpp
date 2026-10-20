#include "client_registry.h"

#include <mutex>

void ClientRegistry::add(int client_id, Queue<ServerMessage>* outgoing) {
    std::lock_guard<std::mutex> lock(mtx);
    queues[client_id] = outgoing;
}

void ClientRegistry::remove(int client_id) {
    std::lock_guard<std::mutex> lock(mtx);
    queues.erase(client_id);
}

void ClientRegistry::send(int client_id, const ServerMessage& message) {
    std::lock_guard<std::mutex> lock(mtx);

    auto it = queues.find(client_id);
    if (it == queues.end()) {
        return;
    }

    try {
        it->second->push(message);
    } catch (const ClosedQueue&) {
        // La cola del cliente ya esta cerrada: se esta desconectando.
    }
}
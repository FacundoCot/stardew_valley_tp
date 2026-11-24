#include "client_registry.h"

#include <mutex>

void ClientRegistry::add(int client_id, Queue<ServerMessage>* outgoing) {
    std::lock_guard<std::mutex> lock(mtx);
    queues[client_id] = outgoing;
}

void ClientRegistry::remove(int client_id) {
    std::lock_guard<std::mutex> lock(mtx);
    queues.erase(client_id);
    usernames_by_client.erase(client_id);
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

bool ClientRegistry::register_username(
    int client_id, const std::string& username) {

    std::lock_guard<std::mutex> lock(mtx);

    if (username.empty() || queues.count(client_id) == 0) {
        return false;
    }

    auto it = usernames_by_client.find(client_id);

    if (it != usernames_by_client.end()) {
        return it->second == username;
    }

    for (const auto& [id, name] : usernames_by_client) {
        if (name == username) {
            return false;
        }
    }

    usernames_by_client[client_id] = username;

    return true;
}
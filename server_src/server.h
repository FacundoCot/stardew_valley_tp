#ifndef SERVER_H
#define SERVER_H

#include <memory>
#include <thread>

#include "../common_src/queue.h"
#include "../common_src/socket.h"

#include "acceptor.h"
#include "client_action.h"
#include "client_registry.h"
#include "game_manager.h"

class Server {
private:
    // El orden de declaracion importa: cada miembro se construye en este
    // orden y los que vienen despues reciben referencias a los anteriores.
    Queue<std::shared_ptr<ClientAction>> actions;
    ClientRegistry registry;
    GameManager manager;
    Acceptor acceptor;
    std::thread gameloop;

    void game_loop();

public:
    explicit Server(Socket&& skt);

    // Server tiene threads y referencias internas: no se copia ni se mueve.
    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;

    void run();
};

#endif
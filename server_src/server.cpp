#include "server.h"

#include <exception>
#include <iostream>
#include <utility>

#include "../common_src/map.h"

Server::Server(Socket&& skt):
        actions(),
        registry(),
        manager(registry, Map(20, 20)),
        acceptor(std::move(skt), actions, registry) {}

void Server::game_loop() {
    try {
        while (true) {
            // pop() se bloquea hasta que llega un pedido. Cuando run()
            // cierra la cola, pop() lanza ClosedQueue y el loop termina.
            std::shared_ptr<ClientAction> action = actions.pop();

            try {
                action->execute(manager);
            } catch (const std::exception& err) {
                // Un pedido malo no debe tirar abajo a todo el servidor.
                std::cerr << "Error procesando accion: " << err.what() << "\n";
            }
        }
    } catch (const ClosedQueue&) {
        // El servidor se esta apagando.
    }
}

void Server::run() {
    acceptor.start();
    gameloop = std::thread(&Server::game_loop, this);

    char command;
    while (std::cin >> command) {
        if (command == 'q') {
            break;
        }
    }

    // 1. Dejar de aceptar clientes y cerrar sus handlers.
    //    acceptor.join() vuelve cuando clean() ya destruyo a todos.
    acceptor.stop();
    acceptor.join();

    // 2. Cerrar la cola: el gameloop sale de pop() y termina.
    actions.close();
    gameloop.join();
}
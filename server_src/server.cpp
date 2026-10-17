#include "server.h"

#include <iostream>
#include <utility>

Server::Server(Socket&& skt): actions(), acceptor(std::move(skt), actions) {}

void Server::run() {
    acceptor.start();

    char command;
    while (std::cin >> command) {
        if (command == 'q') {
            break;
        }
    }

    acceptor.stop();
    acceptor.join();

    actions.close();
}

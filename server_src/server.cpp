#include "server.h"

#include <iostream>
#include <utility>
// include de map.h

Server::Server(Socket&& skt):
        registry(),
        menu_games_monitor(Map(20, 20), registry),
        acceptor(std::move(skt), menu_games_monitor, registry) 
{}

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
}

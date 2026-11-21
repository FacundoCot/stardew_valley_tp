#include "server.h"

#include <iostream>
#include <utility>
Server::Server(Socket&& skt, WorldCatalog& catalog, const GameConfig& config) : 
      registry(),
      menu_games_monitor(catalog, registry, config),
      acceptor(std::move(skt), menu_games_monitor, registry) {}

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

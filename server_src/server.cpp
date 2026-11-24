#include "server.h"

#include <iostream>
#include <utility>
Server::Server(Socket&& skt, WorldCatalog& catalog, const GameConfig& config,
               const EnemyConfig& enemy_config) :
      registry(),
      menu_games_monitor(catalog, registry, config, enemy_config),
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

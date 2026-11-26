#ifndef SERVER_H
#define SERVER_H

#include "client_registry.h"
#include "../common_src/socket.h"
#include "../common_src/queue.h"
#include "menu_games_monitor.h"
#include "acceptor.h"
#include "world_catalog.h"
#include "game_config.h"
#include "enemy_config.h"

#include <memory>

class Server {
private:
  Queue<std::shared_ptr<ClientAction>> actions;
  ClientRegistry registry;
  MenuGamesMonitor menu_games_monitor;
  Acceptor acceptor;


public:
  Server(Socket&& skt, WorldCatalog& catalog, const GameConfig& config,
         const EnemyConfig& enemy_config);
  void run();
};

#endif

#include <iostream>

#include "common_src/socket.h"
#include "server.h"
#include "world_catalog.h"
#include "game_config.h"
#include "enemy_config.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Uso: " << argv[0] << " <servername>\n";
        return 1;
    }
    try {
        const char* servername = argv[1];
        const GameConfig config = GameConfig::load("config/game.toml");
        const EnemyConfig enemy_config = EnemyConfig::load("config/enemy.toml");

        WorldCatalog catalog;
        Server srv = Server(Socket(servername), catalog, config, enemy_config);

        srv.run();
        return 0;

    } catch (const std::exception& err) {
        std::cerr << err.what() << "\n";
        return 1;
    }
}
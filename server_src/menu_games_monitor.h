#ifndef MENU_GAMES_MONITOR_H
#define MENU_GAMES_MONITOR_H

#include <map>
#include <memory>
#include <mutex>
#include <string>

#include "../common_src/messages.h"
#include "game.h" 
#include "server_structs.h"
#include "save_disc.h"
#include "world_catalog.h"

class MenuGamesMonitor {
private:
    std::mutex mtx;
    WorldCatalog& catalog;
    ClientRegistry& registry;
    SaveDisc save_disc;
    std::map<std::string, std::unique_ptr<GameRuntime>> games;
    // Partidas guardadas que todavia no se cargaron: el mundo no se construye
    // hasta que alguien las retoma.
    std::map<std::string, GameSaveData> saved_games;

    int next_game_code = 1;

    // Carga una partida guardada y la deja en 'games'. Construye el mundo recien aca.
    GameRuntime& load_game_unlocked(const std::string& game_code);

public:
    explicit MenuGamesMonitor(WorldCatalog& catalog,
                              ClientRegistry& registry);

    MenuResult create_game(int client_id,
                       const std::string& world_name,
                       const std::string& game_name,
                       const std::string& player_name);

    MenuResult join_game(int client_id,
                        const std::string& game_code,
                        const std::string& player_name);

    ServerMessage list_games();
    };

#endif
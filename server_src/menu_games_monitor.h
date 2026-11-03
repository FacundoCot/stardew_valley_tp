#ifndef MENU_GAMES_MONITOR_H
#define MENU_GAMES_MONITOR_H

#include <map>
#include <memory>
#include <mutex>
#include <string>

#include "../common_src/map.h"
#include "../common_src/messages.h"
#include "game.h" 
#include "server_structs.h"

class MenuGamesMonitor {
private:
    std::mutex mtx;
    Map world_map;
    ClientRegistry& registry;
    std::map<std::string, std::unique_ptr<GameRuntime>> games;

    int next_game_code = 1;

public:
    explicit MenuGamesMonitor(Map world_map, 
                              ClientRegistry& registry);

    MenuResult create_game(int client_id,
                       const std::string& game_name,
                       const std::string& player_name);

MenuResult join_game(int client_id,
                     const std::string& game_code,
                     const std::string& player_name);
};

#endif
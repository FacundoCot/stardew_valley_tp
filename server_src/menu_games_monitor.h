#ifndef MENU_GAMES_MONITOR_H
#define MENU_GAMES_MONITOR_H

#include <map>
#include <memory>
#include <mutex>
#include <string>

#include "../common_src/messages.h"
// faltan los includes de map.h y de game.h 

class MenuGamesMonitor {
private:
    std::mutex mtx;
    Map world_map;
    std::map<std::string, std::shared_ptr<Game>> games;

    int next_game_code = 1;

public:
    explicit MenuGamesMonitor(Map world_map);

    MenuResult create_game(const std::string& game_name,
                           const std::string& player_name);

    MenuResult join_game(const std::string& game_code,
                         const std::string& player_name);
};

#endif
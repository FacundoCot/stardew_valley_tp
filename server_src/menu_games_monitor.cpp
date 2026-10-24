#include "menu_games_monitor.h"

#include <utility>

MenuGamesMonitor::MenuGamesMonitor(Map world_map_param):
    world_map(std::move(world_map_param)) {}

MenuResult MenuGamesMonitor::create_game(int client_id,
                                        const std::string& game_name,
                                        const std::string& player_name){
        
    (void)game_name;
    (void)player_name;

    std::lock_guard<std::mutex> lock(mtx);
    
    auto game = std::make_shared<Game>(world_map);

    if (!game->add_player_auto(client_id))
    {
        return MenuResult{false, "No hay lugar en el mapa", "", nullptr};
    }
    
    std::string code = std::to_string(next_game_code++);
    games[code] = game;
    return MenuResult{true, "", code, game};
}

MenuResult MenuGamesMonitor::join_game(int client_id,
                                       const std::string& game_code,
                                       const std::string& player_name) {
    (void)player_name;  // se guardara cuando Game tenga nombres

    std::lock_guard<std::mutex> lock(mtx);

    auto it = games.find(game_code);
    if (it == games.end()) {
        return MenuResult{false, "No existe una partida con ese codigo", "", nullptr};
    }

    if (!it->second->add_player_auto(client_id)) {
        return MenuResult{false, "No hay lugar en la partida", "", nullptr};
    }

    return MenuResult{true, "", game_code, it->second};
}
#include "menu_games_monitor.h"

#include <memory>
#include <utility>

#include "client_action.h"

MenuGamesMonitor::MenuGamesMonitor(Map world_map_param, ClientRegistry& registry):
    world_map(std::move(world_map_param)),
    registry(registry),
    save_disc("saves") 
{
    for (const std::string& code : save_disc.list_saves()) {
        GameSaveData data = save_disc.load(code);

        auto game = std::make_shared<Game>(world_map);
        game->restore_save_data(data);

        games[code] = std::make_unique<GameRuntime>(registry, save_disc, code, game,
                data.game_name, data.owner_name, data.multiplayer);

        int saved_code = std::stoi(code);
        if (saved_code >= next_game_code) {
            next_game_code = saved_code + 1;
        }
    }
}

GameRuntime::GameRuntime(ClientRegistry& registry,
                         SaveDisc& save_disc,
                         std::string game_code_param,
                         std::shared_ptr<Game> game_param,
                         std::string game_name_param,
                         std::string owner_name_param,
                         bool multiplayer_param):
        game(std::move(game_param)),
        game_code(std::move(game_code_param)),
        game_name(std::move(game_name_param)),
        owner_name(std::move(owner_name_param)),
        multiplayer(multiplayer_param),
        manager(registry, game, save_disc, game_code,
                game_name, owner_name, multiplayer),
        loop(manager, game->get_actions_queue()) {

    loop.start();
}

MenuResult MenuGamesMonitor::create_game(int client_id,
                                        const std::string& game_name,
                                        const std::string& player_name){
        
    (void)game_name;

    std::lock_guard<std::mutex> lock(mtx);
    
    auto game = std::make_shared<Game>(world_map);

    if (!game->add_player_auto(client_id, player_name))
    {
        return MenuResult{false, "No hay lugar en el mapa", "", nullptr};
    }
    
    std::string code = std::to_string(next_game_code++);

    games[code] = std::make_unique<GameRuntime>(registry, save_disc, code, game,
            game_name, player_name, false);

    auto join = std::make_shared<JoinAction>();
    join->set_client_id(client_id);
    game->get_actions_queue().push(join);

    return MenuResult{true, "", code, game};
}

MenuResult MenuGamesMonitor::join_game(int client_id,
                                       const std::string& game_code,
                                       const std::string& player_name) {

    std::lock_guard<std::mutex> lock(mtx);

    auto it = games.find(game_code);
    if (it == games.end()) {
        return MenuResult{false, "No existe una partida con ese codigo", "", nullptr};
    }

    if (!it->second->multiplayer && player_name != it->second->owner_name) {
        return MenuResult{false, "Solo el duenio puede entrar a esta partida", "", nullptr};
    }

    auto game = it->second->game;

    if (!game->add_player_auto(client_id, player_name)) {
        return MenuResult{false, "No hay lugar en la partida", "", nullptr};
    }

    auto join = std::make_shared<JoinAction>();
    join->set_client_id(client_id);
    game->get_actions_queue().push(join);

    return MenuResult{true, "", game_code, game};
}

ServerMessage MenuGamesMonitor::list_games() {
    std::lock_guard<std::mutex> lock(mtx);

    ServerMessage msg{};
    msg.type = MessageType::GAMES_LIST;

    for (const auto& [code, runtime] : games) {
        GameInfoForMenu game_info;
        game_info.game_code = code;
        game_info.game_name = runtime->game_name;
        game_info.owner_name = runtime->owner_name;
        game_info.multiplayer = runtime->multiplayer;
        msg.games.push_back(game_info);
    }

    return msg;
}

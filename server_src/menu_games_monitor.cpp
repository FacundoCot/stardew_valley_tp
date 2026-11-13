#include "menu_games_monitor.h"

#include <memory>
#include <stdexcept>
#include <utility>

#include "client_action.h"

MenuGamesMonitor::MenuGamesMonitor(WorldCatalog& catalog_param,
                                   ClientRegistry& registry):
    catalog(catalog_param),
    registry(registry),
    save_disc("saves") 
{
    // Al arrancar solo se leen los datos guardados. No se construye ningun mundo.
    for (const std::string& code : save_disc.list_saves()) {
        GameSaveData data = save_disc.load(code);

        int saved_code = std::stoi(code);
        if (saved_code >= next_game_code) {
            next_game_code = saved_code + 1;
        }
        saved_games.emplace(code, std::move(data));
    }
}

GameRuntime& MenuGamesMonitor::load_game_unlocked(const std::string& game_code) {
    const auto loaded = games.find(game_code);
    if (loaded != games.end()) {
        return *loaded->second;
    }

    const auto saved = saved_games.find(game_code);
    if (saved == saved_games.end()) {
        throw std::invalid_argument("no existe la partida " + game_code);
    }
    const GameSaveData& data = saved->second;

    // TODO: el guardado debe persistir el nombre del mundo.
    auto game = std::make_shared<Game>(catalog.get("por_defecto"));
    game->restore_save_data(data);

    auto runtime = std::make_unique<GameRuntime>(
        registry, game, data.game_name, data.owner_name, data.multiplayer);
    GameRuntime& ref = *runtime;
    games[game_code] = std::move(runtime);
    saved_games.erase(game_code);
    return ref;
}

GameRuntime::GameRuntime(ClientRegistry& registry,
                         std::shared_ptr<Game> game_param,
                         std::string game_name_param,
                         std::string owner_name_param,
                         bool multiplayer_param):
        game(std::move(game_param)),
        game_name(game_name_param),
        owner_name(owner_name_param),
        multiplayer(multiplayer_param),
        manager(registry, game),
        loop(manager, game->get_actions_queue()) {

    loop.start();
}

MenuResult MenuGamesMonitor::create_game(int client_id,
                                        const std::string& world_name,
                                        const std::string& game_name,
                                        const std::string& player_name){
    std::lock_guard<std::mutex> lock(mtx);
    auto game = std::make_shared<Game>(catalog.get(world_name));

    if (!game->add_player_auto(client_id, player_name))
    {
        return MenuResult{false, "No hay lugar en el mapa", "", nullptr};
    }
    
    std::string code = std::to_string(next_game_code++);

    games[code] = std::make_unique<GameRuntime>(registry, game, game_name, player_name, false);

    auto join = std::make_shared<JoinAction>();
    join->set_client_id(client_id);
    game->get_actions_queue().push(join);

    return MenuResult{true, "", code, game};
}

MenuResult MenuGamesMonitor::join_game(int client_id,
                                       const std::string& game_code,
                                       const std::string& player_name) {

    std::lock_guard<std::mutex> lock(mtx);

    // Chequeo de duenio con los datos guardados, antes de cargar el mundo.
    bool multiplayer = false;
    std::string owner_name;
    if (const auto it = games.find(game_code); it != games.end()) {
        multiplayer = it->second->multiplayer;
        owner_name = it->second->owner_name;
    } else if (const auto saved = saved_games.find(game_code); saved != saved_games.end()) {
        multiplayer = saved->second.multiplayer;
        owner_name = saved->second.owner_name;
    } else {
        return MenuResult{false, "No existe una partida con ese codigo", "", nullptr};
    }

    if (!multiplayer && player_name != owner_name) {
        return MenuResult{false, "Solo el duenio puede entrar a esta partida", "", nullptr};
    }

    std::shared_ptr<Game> game;
    try {
        game = load_game_unlocked(game_code).game;
    } catch (const std::exception& err) {
        return MenuResult{false, std::string("No se pudo cargar la partida: ") + err.what(), "", nullptr};
    }

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

    // Las partidas guardadas que todavia no se cargaron tambien se listan.
    for (const auto& [code, data] : saved_games) {
        GameInfoForMenu game_info;
        game_info.game_code = code;
        game_info.game_name = data.game_name;
        game_info.owner_name = data.owner_name;
        game_info.multiplayer = data.multiplayer;
        msg.games.push_back(game_info);
    }

    return msg;
}
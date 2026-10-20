#include "game_manager.h"

#include <utility>

GameManager::GameManager(ClientRegistry& registry_param, Map world_map_param):
        registry(registry_param),
        world_map(std::move(world_map_param)),
        rng(std::random_device{}()) {}

std::string GameManager::generate_code() {
    const std::string letters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::uniform_int_distribution<size_t> pick(0, letters.size() - 1);

    std::string code;
    do {
        code.clear();
        for (int i = 0; i < 4; i++) {
            code += letters[pick(rng)];
        }
    } while (games.count(code) > 0);  // repetir hasta encontrar un codigo libre

    return code;
}

void GameManager::send_error(int client_id, const std::string& text) {
    ServerMessage msg;
    msg.type = MessageType::ERROR;
    msg.error = text;
    registry.send(client_id, msg);
}

void GameManager::send_state(const Game& game) {
    ServerMessage msg;
    msg.type = MessageType::GAME_STATE;
    msg.players = game.snapshot();

    for (int id : game.player_ids()) {
        registry.send(id, msg);
    }
}

void GameManager::create_game(int client_id,
                              const std::string& game_name,
                              const std::string& player_name) {
    (void)game_name;    // se guardara cuando Game tenga nombre
    (void)player_name;  // idem

    if (client_game.count(client_id) > 0) {
        send_error(client_id, "Ya estas en una partida");
        return;
    }

    std::string code = generate_code();
    auto result = games.emplace(code, Game(world_map));
    Game& game = result.first->second;

    if (!game.add_player_auto(client_id)) {
        games.erase(code);
        send_error(client_id, "No hay lugar en el mapa");
        return;
    }

    client_game[client_id] = code;

    ServerMessage msg;
    msg.type = MessageType::GAME_CREATED;
    msg.game_code = code;
    msg.player_id = static_cast<uint32_t>(client_id);
    registry.send(client_id, msg);

    send_state(game);
}

void GameManager::join_game(int client_id,
                            const std::string& game_code,
                            const std::string& player_name) {
    (void)player_name;  // se guardara cuando Game tenga nombres

    if (client_game.count(client_id) > 0) {
        send_error(client_id, "Ya estas en una partida");
        return;
    }

    auto game_it = games.find(game_code);
    if (game_it == games.end()) {
        send_error(client_id, "No existe una partida con ese codigo");
        return;
    }

    Game& game = game_it->second;
    if (!game.add_player_auto(client_id)) {
        send_error(client_id, "No hay lugar en la partida");
        return;
    }

    client_game[client_id] = game_code;

    ServerMessage msg;
    msg.type = MessageType::GAME_JOINED;
    msg.game_code = game_code;
    msg.player_id = static_cast<uint32_t>(client_id);
    registry.send(client_id, msg);

    send_state(game);
}

void GameManager::move_player(int client_id, Direction direction) {
    auto code_it = client_game.find(client_id);
    if (code_it == client_game.end()) {
        return;  // el cliente no esta en ninguna partida
    }

    auto game_it = games.find(code_it->second);
    if (game_it == games.end()) {
        return;
    }

    // Si el movimiento es invalido, Game no cambia nada y no se manda estado.
    if (game_it->second.move_player(client_id, direction)) {
        send_state(game_it->second);
    }
}

void GameManager::disconnect_client(int client_id) {
    auto code_it = client_game.find(client_id);
    if (code_it == client_game.end()) {
        return;
    }

    std::string code = code_it->second;
    client_game.erase(code_it);

    auto game_it = games.find(code);
    if (game_it == games.end()) {
        return;
    }

    Game& game = game_it->second;
    game.remove_player(client_id);

    if (game.is_empty()) {
        games.erase(game_it);  // nadie mas en la partida: se borra
        return;
    }

    send_state(game);
}
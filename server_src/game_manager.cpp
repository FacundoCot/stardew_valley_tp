#include "game_manager.h"

#include <utility>

GameManager::GameManager(ClientRegistry& registry_param,
                         std::shared_ptr<Game> game_param):
        registry(registry_param),
        game(std::move(game_param)) {}

void GameManager::send_state() {
    ServerMessage msg;
    msg.type = MessageType::GAME_STATE;
    msg.players = game->snapshot();

    for (int id : game->player_ids()) {
        registry.send(id, msg);
    }
}

void GameManager::move_player(int client_id, Direction direction) {
    // Si el movimiento es invalido, Game no cambia nada y no se manda estado.
    if (game->move_player(client_id, direction)) {
        send_state();
    }
}

void GameManager::disconnect_client(int client_id) {
    game->remove_player(client_id);

    if (!game->is_empty()) {
        send_state();
    }
}
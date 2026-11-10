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
    msg.enemies = game->enemy_snapshot();

    for (int id : game->player_ids()) {
        registry.send(id, msg);
    }
}

void GameManager::attack_player(int client_id) {
    if (game->attack(client_id)) {
        send_state();
    }
}

void GameManager::tick(float dt) {
    TimeState before = game->time_state();

    if (game->update(dt)) {
        send_state();
    }

    // Como update() se ejecuta varias veces por segundo, no queremos
    // enviar el estado del reloj en cada tick, sino solo cuando cambia
    // la hora, el minuto, el día o el estado de finalización.
    // Para eso, comparamos el estado antes y después de actualizar el juego.
    TimeState after = game->time_state();
    if (before.day != after.day ||
        before.hour != after.hour ||
        before.minute != after.minute ||
        before.finished != after.finished) {
        send_time_global();
    }
}

void GameManager::move_player(int client_id, Direction direction) {
    // Si el movimiento es invalido, Game no cambia nada y no se manda estado.
    if (game->move_player(client_id, direction)) {
        send_state();
    }
}

void GameManager::join_player(int client_id) {
    send_state();
    send_inventory(client_id);
    send_time(client_id);
}

void GameManager::disconnect_client(int client_id) {
    if (game->remove_player(client_id) && !game->is_empty()) {
        send_state();
    }
}

void GameManager::send_inventory(int client_id) {
    ServerMessage msg{};
    msg.type = MessageType::INVENTORY_STATE;
    msg.inventory = game->inventory_state(client_id);

    registry.send(client_id, msg);
}

void GameManager::send_time(int client_id) {
    ServerMessage msg{};
    msg.type = MessageType::TIME_STATE;
    msg.time = game->time_state();

    registry.send(client_id, msg);
}

void GameManager::send_time_global() {
    ServerMessage msg{};
    msg.type = MessageType::TIME_STATE;
    msg.time = game->time_state();

    for (int id : game->player_ids()) {
        registry.send(id, msg);
    }
}


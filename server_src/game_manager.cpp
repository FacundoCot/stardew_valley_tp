#include "game_manager.h"

#include <utility>
#include <sstream>

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

void GameManager::send_chat_global(const std::string& name, const std::string& text) {

    ServerMessage msg{};
    msg.type = MessageType::CHAT_MESSAGE;
    msg.chat.kind = ChatKind::PUBLIC;
    msg.chat.sender = name;
    msg.chat.text = text;

    for (int id : game->player_ids()) {
        registry.send(id, msg);
    }
}

void GameManager::send_system_message(int client_id, const std::string& text) {
    ServerMessage msg{};

    msg.type = MessageType::CHAT_MESSAGE;
    msg.chat.kind = ChatKind::SYSTEM;
    msg.chat.sender = "Server";
    msg.chat.text = text;

    registry.send(client_id, msg);
}

void GameManager::send_chat_private(int client_id, const std::string& sender, const std::string& text) {
    std::istringstream stream(text);

    std::string receiver;
    stream >> receiver;

    if (receiver.size() <= 1) {
        send_system_message(client_id, "invalid receiver");
        return;
    }

    receiver.erase(0, 1); // Quitamos el @

    std::string message;
    std::getline(stream >> std::ws, message);

    if (message.empty()) {
        send_system_message(client_id, "empty message");
        return;
    }

    int receiver_id;

    if (!game->name_to_client_id(receiver_id,receiver)) {
        send_system_message(client_id, "disconnected or non-existent player");
        return;
    }

    ServerMessage msg{};
    msg.type = MessageType::CHAT_MESSAGE;
    msg.chat.kind = ChatKind::PRIVATE;
    msg.chat.sender = sender;
    msg.chat.text = message;

    registry.send(receiver_id, msg);
}

void GameManager::handle_command(int client_id, const std::string& text) {
    std::istringstream stream(text);

    std::string command;
    stream >> command;

    if (command == "/consultar_catalogo") {
        std::string extra;

        if (stream >> extra) {
            send_system_message(client_id, "Uso: /consultar_catalogo");
            return;
        }

        // falta implementacion
        send_system_message(client_id, "the implementation of the transaction is not yet available");
        return;
    }

    if (command == "/comprar" || command == "/vender") {
        std::string product;
        std::string extra;
        int quantity = 0;

        if (!(stream >> product >> quantity) || quantity <= 0 || (stream >> extra)) {
            send_system_message(client_id, "Use: /comprar producto cantidad o /vender producto cantidad");
            return;
        }

        // falta implementacion
        send_system_message(client_id, "the implementation of the transaction is not yet available");
        return;
    }

    send_system_message(client_id, "Comando desconocido");
}

void GameManager::send_chat(int client_id, const std::string& text) {
    std::string name;

    if (!game->player_name(client_id, name)) {
        return;
    }

    if (text.empty()) {
        return;
    }

    if (text[0] == '@') {
        send_chat_private(client_id, name, text);
        return;
    }

    if (text[0] == '/') {
        handle_command(client_id, text);
        return;
    }

    send_chat_global(name, text);
}

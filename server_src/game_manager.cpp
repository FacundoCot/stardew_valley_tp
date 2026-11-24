#include "game_manager.h"

#include <utility>
#include <sstream>

GameManager::GameManager(ClientRegistry& registry_param,
                         std::shared_ptr<Game> game_param,
                         SaveDisc& save_disc_param,
                         const std::string& game_code_param,
                         const std::string& game_name_param,
                         const std::string& owner_name_param,
                         const std::string& world_name_param,
                         bool multiplayer_param):
        registry(registry_param),
        game(std::move(game_param)),
        save_disc(save_disc_param),
        game_code(game_code_param),
        game_name(game_name_param),
        owner_name(owner_name_param),
        world_name(world_name_param),
        multiplayer(multiplayer_param) {}

void GameManager::send_state() {
    for (int id : game->player_ids()) {
        ServerMessage state = game->game_state(id);
        registry.send(id, state);
    }
}

void GameManager::attack_player(int client_id) {
    if (game->attack(client_id)) {
        send_state();
    }
}

void GameManager::tick(float dt) {
    game->update(dt);
    send_state();
}

void GameManager::move_player(int client_id, Direction direction) {
    // Si el movimiento es invalido, Game no cambia nada y no se manda estado.
    if (game->move_player(client_id, direction)) {
        send_state();
    }
}

void GameManager::join_player(int /*no se necesita el client_id*/) {
    send_state();
}

void GameManager::disconnect_client(int client_id) {
    if (game->remove_player(client_id) && !game->is_empty()) {
        send_state();
        try_start_next_day();
    }
}

/*
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
*/

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

    auto catalog = game->get_merchant_catalog(client_id);

    if (!catalog) {
        send_system_message(client_id, "No estas interactuando con ningun comerciante");
        return;
    }

    send_system_message(client_id, *catalog);
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

        auto merchant = game->get_active_merchant(client_id);

        if (!merchant) {
            send_system_message(
                client_id, "You have not selected any merchant"
            );
            return;
        }

        bool success;

        if (command == "/comprar") {
            success = game->buy_item(client_id, *merchant, product, quantity);
        } else {
            success = game->sell_item(client_id, *merchant, product, quantity);
        }

        if (!success) {
            send_system_message(client_id, "The transaction failed");
            return;
        }

        if (command == "/comprar") {
            send_system_message(client_id, "buy succed");
        } else {
            send_system_message(client_id, "sell succed");
        }

        send_state();
        return;
    }

    send_system_message(client_id, "Unknown command");
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

void GameManager::try_start_next_day() {
    if (!game->start_next_day_game()) {
        return;
    }

    try {
        GameSaveData data = game->get_save_data(
                game_code, game_name, owner_name, world_name, game->is_multiplayer());

        save_disc.save(data);

    } catch (const std::exception& e) {
        for (int id : game->player_ids()) {
            send_system_message(id, "Error al guardar la partida");
        }
    }
    send_state();
}

void GameManager::confirm_next_day(int client_id) {
    if (!game->confirm_next_day(client_id)) {
        return;
    }
    try_start_next_day();
}

void GameManager::select_slot(int client_id, int slot_index) {
    if (game->select_slot(client_id, slot_index)) {
        send_state();
    }
}

void GameManager::enable_multiplayer(int client_id) {
    game->enable_multiplayer();

    ServerMessage msg{};
    msg.type = MessageType::MULTIPLAYER_ENABLED;
    msg.game_code = game_code;

    registry.send(client_id, msg);
}

void GameManager::interact_merchant(int client_id, MerchantType merchant) {
    if (!game->interact_with_merchant(client_id, merchant)) {
        send_system_message(client_id,"You can't interact with this NPC");
        return;
    }

    std::string name;

    switch (merchant) {
        case MerchantType::PIERRE:
            name = "Pierre";
            break;
        case MerchantType::ROBIN:
            name = "Robin";
            break;
        case MerchantType::MARNIE:
            name = "Marnie";
            break;
        case MerchantType::CLINT:
            name = "Clint";
            break;
    }

    ServerMessage message{};
    message.type = MessageType::CHAT_MESSAGE;
    message.chat.kind = ChatKind::NPC;
    message.chat.sender = name;
    message.chat.text =
        "Hello! Commands: /consultar_catalogo, /comprar producto cantidad, /vender producto cantidad";

    registry.send(client_id, message);
}

void GameManager::till_soil(int client_id, int x, int y) {
    if (game->till_soil(client_id, x, y)) {
        send_state();
    }
}

void GameManager::plant_seed(int client_id, int x, int y) {
    if (game->plant_seed(client_id, x, y)) {
        send_state();
    }
}

void GameManager::water_crop(int client_id, int x, int y) {
    if (game->water_crop(client_id, x, y)) {
        send_state();
    }
}

void GameManager::harvest_crop(int client_id, int x, int y) {
    if (game->harvest_crop(client_id, x, y)) {
        send_state();
    }
}
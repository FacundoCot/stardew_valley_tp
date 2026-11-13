#ifndef GAME_MANAGER_H
#define GAME_MANAGER_H

#include <string>

#include "../common_src/map.h"
#include "../common_src/messages.h"

#include "client_registry.h"
#include "game.h"
#include "game_action_handler.h"
#include "save_disc.h"

class GameManager : public GameActionHandler {
private:
    ClientRegistry& registry;
    std::shared_ptr<Game> game;

    SaveDisc& save_disc;
    std::string game_code;
    std::string game_name;
    std::string owner_name;
    bool multiplayer;

    void send_state();

    void send_chat_global(const std::string& name, const std::string& text);

    void send_chat_private(const int client_id, const std::string& sender, const std::string& text);

    void handle_command(const int client_id, const std::string& text);

    void send_system_message(int client_id, const std::string& text);

    void try_start_next_day();
public:
    explicit GameManager(ClientRegistry& registry,
            std::shared_ptr<Game> game,
            SaveDisc& save_disc,
            const std::string& game_code,
            const std::string& game_name,
            const std::string& owner_name,
            bool multiplayer);

    void move_player(int client_id, Direction direction) override;

    void join_player(int client_id) override;

    void attack_player(int client_id) override;

    void tick(float dt);

    void disconnect_client(int client_id) override;

    void send_inventory(int client_id);

    void send_time(int client_id);

    void send_time_global();

    void send_chat(int client_id, const std::string& text) override;

    void confirm_next_day(int client_id) override;
};

#endif
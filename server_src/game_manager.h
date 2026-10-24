#ifndef GAME_MANAGER_H
#define GAME_MANAGER_H

#include <string>

#include "../common_src/map.h"
#include "../common_src/messages.h"

#include "client_registry.h"
#include "game.h"
#include "game_action_handler.h"

class GameManager : public GameActionHandler {
private:
     ClientRegistry& registry;
    std::shared_ptr<Game> game;

    void send_state();

public:
    GameManager(ClientRegistry& registry, std::shared_ptr<Game> game);

    void move_player(int client_id, Direction direction) override;

    void disconnect_client(int client_id) override;
};

#endif
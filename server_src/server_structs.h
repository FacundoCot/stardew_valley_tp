#ifndef SERVER_STRUCTS_H
#define SERVER_STRUCTS_H

#include <memory>

#include "game.h"
#include "game_manager.h"
#include "game_loop.h"
#include "client_registry.h"

struct GameRuntime {
    std::shared_ptr<Game> game;
    GameManager manager;
    GameLoop loop;

    GameRuntime(ClientRegistry& registry,
                std::shared_ptr<Game> game_param);
};

#endif

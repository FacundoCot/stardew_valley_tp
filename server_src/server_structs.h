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

enum class Quality {
    BASE,
    GOOD,
    EXCELLENT,
    INCREDIBLE,
    PERFECT
};

enum class ItemType {
    // Herramientas
    PICKAXE,
    WATERING_CAN,
    SHOVEL,
    SWORD,

    // Cultivos
    STRAWBERRY,
    ONION,
    POTATO,
    PUMPKIN,

    // Semillas
    STRAWBERRY_SEED,
    ONION_SEED,
    POTATO_SEED,
    PUMPKIN_SEED,

    // Minerales
    COAL,
    LAPIS_LAZULI,
    SILVER,
    GOLD,

    // Productos animales
    MILK,
    EGG,

    // Otros
    BED,
    CHEST
};

#endif

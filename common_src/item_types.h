#ifndef ITEM_TYPES_H
#define ITEM_TYPES_H

#include <cstdint>

enum class Quality : uint8_t {
    BASE,
    GOOD,
    EXCELLENT,
    INCREDIBLE,
    PERFECT
};

enum class ItemType : uint8_t {
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
    CHEST,

    // Decoracion
    CHAIR,
    BARREL_CLOSED,
    BARREL_SWORDS,
    BARREL_WATER,
    CRATE_1,
    CRATE_2,
    FLOWERS_1,
    FLOWERS_2,
    PICTURE_1,
    PICTURE_2,
    WELL,
    STATUE,
    BEAM
};

enum class RemoveMode {
    ONE,
    HALF,
    ALL
};

#endif
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
    CHEST
};

enum class RemoveMode {
    ONE,
    HALF,
    ALL
};

#endif
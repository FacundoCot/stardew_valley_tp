#ifndef MAP_DEFINITIONS_H
#define MAP_DEFINITIONS_H

#include <string>

#include <QColor>

const int TILE_SIZE = 16;

const int CAVE_LEVELS = 10;

const int ZONE_FARM = 0;
const int ZONE_VILLAGE = 1;
const int ZONE_FIRST_CAVE = 2; 
const int ZONE_HOUSE = 12;
const int ZONE_COMMUNITY_CENTER = ZONE_HOUSE + 1;
const int ZONE_COUNT = ZONE_COMMUNITY_CENTER + 1;

struct ElementType {
    std::string id; // nombre archivo
    std::string name; // nombre en editor
    QColor color;
    bool unique; // por zona
};

const int ELEMENT_COUNT = 24;

const QColor PORTAL_COLOR(70, 130, 255, 190);
const QColor BED_COLOR(60, 180, 90, 190);
const QColor NPC_COLOR(240, 160, 40, 190);
const QColor ENEMY_SPAWN_COLOR(220, 60, 60, 190);

const QColor COAL_COLOR(60, 60, 60, 220);
const QColor LAPIS_COLOR(40, 70, 170, 200);
const QColor SILVER_COLOR(200, 200, 210, 200);
const QColor GOLD_COLOR(240, 200, 40, 200);

const QColor SPIKES_COLOR(150, 70, 200, 190);
const QColor FLAMETHROWER_COLOR(220, 70, 150, 190);

const ElementType ELEMENT_TYPES[ELEMENT_COUNT] = {
    {"road", "Camino (granja <-> pueblo)", PORTAL_COLOR, true},
    {"dungeon_door", "Entrada a la caverna", PORTAL_COLOR, true},
    {"house_door", "Puerta de la casa", PORTAL_COLOR, true},
    {"center_door", "Puerta del centro comunitario", PORTAL_COLOR, true},
    {"stair_down", "Escalera abajo", PORTAL_COLOR, true},
    {"stair_up", "Escalera arriba", PORTAL_COLOR, true},
    {"dungeon_exit", "Salida de la caverna", PORTAL_COLOR, true},
    {"house_exit", "Salida de la casa", PORTAL_COLOR, true},
    {"center_exit", "Salida del centro comunitario", PORTAL_COLOR, true},
    {"bed", "Cama", BED_COLOR, true},
    {"marnie", "Marnie", NPC_COLOR, true},
    {"clint", "Clint", NPC_COLOR, true},
    {"pierre", "Pierre", NPC_COLOR, true},
    {"robin", "Robin", NPC_COLOR, true},
    {"enemy", "Spawn de enemigo", ENEMY_SPAWN_COLOR, false},
    {"ore_coal", "Mineral: carbón", COAL_COLOR, false},
    {"ore_lapis", "Mineral: lapislázuli", LAPIS_COLOR, false},
    {"ore_silver", "Mineral: plata", SILVER_COLOR, false},
    {"ore_gold", "Mineral: oro", GOLD_COLOR, false},
    {"trap_spikes", "Trampa: pinches en el suelo", SPIKES_COLOR, false},
    {"trap_flame_up", "Trampa: lanzallamas hacia arriba", FLAMETHROWER_COLOR, false},
    {"trap_flame_down", "Trampa: lanzallamas hacia abajo", FLAMETHROWER_COLOR, false},
    {"trap_flame_left", "Trampa: lanzallamas hacia la izquierda", FLAMETHROWER_COLOR, false},
    {"trap_flame_right", "Trampa: lanzallamas hacia la derecha", FLAMETHROWER_COLOR, false},
};

#endif

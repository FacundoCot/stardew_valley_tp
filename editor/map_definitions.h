#ifndef MAP_DEFINITIONS_H
#define MAP_DEFINITIONS_H

#include <string>

#include <QColor>

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

const int ELEMENT_COUNT = 15;

const QColor PORTAL_COLOR(70, 130, 255, 190);
const QColor BED_COLOR(60, 180, 90, 190);
const QColor NPC_COLOR(240, 160, 40, 190);
const QColor ENEMY_SPAWN_COLOR(220, 60, 60, 190);

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
    {"bed", "Cama", BED_COLOR, false},
    {"marnie", "Marnie", NPC_COLOR, true},
    {"clint", "Clint", NPC_COLOR, true},
    {"pierre", "Pierre", NPC_COLOR, true},
    {"robin", "Robin", NPC_COLOR, true},
    {"enemy", "Spawn de enemigo", ENEMY_SPAWN_COLOR, false},
};

#endif

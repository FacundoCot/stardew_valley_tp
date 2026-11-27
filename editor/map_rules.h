#ifndef MAP_RULES_H
#define MAP_RULES_H

#include <string>
#include <unordered_map>
#include <unordered_set>

#include "map_definitions.h"

const int GOLD_FIRST_LEVEL = 5;

const std::unordered_map<int, std::string> ZONE_NAMES = {
    {ZONE_FARM, "Granja"},
    {ZONE_VILLAGE, "Pueblo"},
    {ZONE_HOUSE, "Casa"},
    {ZONE_COMMUNITY_CENTER, "Centro comunitario"},
};

const std::unordered_map<int, std::unordered_set<std::string>> REQUIRED_ELEMENTS = {
    {ZONE_FARM, {"road", "house_door"}},
    {ZONE_VILLAGE, {"road", "dungeon_door", "center_door", "marnie", "clint", "pierre", "robin"}},
    {ZONE_FIRST_CAVE, {"dungeon_exit", "stair_down"}},
    {ZONE_FIRST_CAVE + 1, {"dungeon_exit", "stair_down", "stair_up"}},
    {ZONE_FIRST_CAVE + 2, {"dungeon_exit", "stair_down", "stair_up"}},
    {ZONE_FIRST_CAVE + 3, {"dungeon_exit", "stair_down", "stair_up"}},
    {ZONE_FIRST_CAVE + 4, {"dungeon_exit", "stair_down", "stair_up"}},
    {ZONE_FIRST_CAVE + 5, {"dungeon_exit", "stair_down", "stair_up"}},
    {ZONE_FIRST_CAVE + 6, {"dungeon_exit", "stair_down", "stair_up"}},
    {ZONE_FIRST_CAVE + 7, {"dungeon_exit", "stair_down", "stair_up"}},
    {ZONE_FIRST_CAVE + 8, {"dungeon_exit", "stair_down", "stair_up"}},
    {ZONE_FIRST_CAVE + 9, {"dungeon_exit", "stair_up"}},
    {ZONE_HOUSE, {"house_exit", "bed"}},
    {ZONE_COMMUNITY_CENTER, {"center_exit"}},
};

// repetibles en cavernas
const std::unordered_set<std::string> CAVE_ONLY_ELEMENTS = {
    "enemy", "ore_coal", "ore_lapis", "ore_silver", "ore_gold", "trap_spikes",
    "trap_flame_up", "trap_flame_down", "trap_flame_left", "trap_flame_right",
};

// no pueden estar tapados
const std::unordered_set<std::string> FLOOR_ELEMENTS = {
    "road", "dungeon_door", "house_door", "center_door", "stair_down", "stair_up", 
    "dungeon_exit", "house_exit", "center_exit", "enemy", "marnie", "clint", "pierre", 
    "robin", "trap_spikes"
};

#endif

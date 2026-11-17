#ifndef SAVE_DATA_STRUCTS_H
#define SAVE_DATA_STRUCTS_H

#include <string>
#include <vector>

#include "../common_src/item_types.h"
#include "../common_src/zone.h"
#include "../common_src/character_appearance.h"

struct ItemSaveData {
    ItemType type;
    Quality quality;
    int quantity;
    int index;
};

struct PlayerSaveData {
    std::string name;
    CharacterAppearance appearance;
    float x;
    float y;
    int health;
    int energy;
    int coins;
    ZoneId zone;
    int bed_index;
    int selected_slot = 0;
    std::vector<ItemSaveData> inventory;
};

struct GameSaveData {
    std::string game_code;
    std::string game_name;
    std::string owner_name;
    std::string world_name; 
    bool multiplayer;
    int day;
    std::vector<PlayerSaveData> players;
};

#endif

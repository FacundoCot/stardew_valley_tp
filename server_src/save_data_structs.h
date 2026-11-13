#ifndef SAVE_DATA_STRUCTS_H
#define SAVE_DATA_STRUCTS_H

#include <string>
#include <vector>

#include "../common_src/item_types.h"

struct ItemSaveData {
    ItemType type;
    Quality quality;
    int quantity;
    int index;
};

struct PlayerSaveData {
    std::string name;
    float x;
    float y;
    int health;
    int coins;
    std::vector<ItemSaveData> inventory;
};

struct GameSaveData {
    std::string game_code;
    std::string game_name;
    std::string owner_name;
    bool multiplayer;
    int day;
    std::vector<PlayerSaveData> players;
};

#endif

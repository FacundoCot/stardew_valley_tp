#ifndef SAVE_DATA_STRUCTS_H
#define SAVE_DATA_STRUCTS_H

#include <string>
#include <vector>
#include <optional>

#include "../common_src/item_types.h"
#include "../common_src/zone.h"
#include "../common_src/character_appearance.h"

struct ItemSaveData {
    ItemType type;
    Quality quality;
    int quantity;
    int index;
};

struct DecorationSaveData {
    int x;
    int y;
    ItemType type;
};

struct ChestSaveData {
    int x;
    int y;
    std::vector<ItemSaveData> inventory;
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

struct CropSaveData {
    ItemType type;

    int required_waterings;
    int growth_days;
    int regrowth_days;
    int tolerated_missed_waterings;

    int days_grown;
    int waterings_today;
    int last_watering_window;
    int missed_waterings;
    int watering_delay_minutes;
    int ready_days;

    bool ready_to_harvest;
};

struct FarmPlotSaveData {
    int x;
    int y;
    bool has_crop = false;
    CropSaveData crop{};
};

struct GameSaveData {
    std::string game_code;
    std::string game_name;
    std::string owner_name;
    std::string world_name; 
    bool multiplayer;
    int day;
    std::vector<PlayerSaveData> players;
    std::vector<FarmPlotSaveData> farm_plots;
    std::vector<DecorationSaveData> decorations;
    std::vector<ChestSaveData> chests;
};

#endif

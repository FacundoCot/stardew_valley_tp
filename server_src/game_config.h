#ifndef GAME_CONFIG_H
#define GAME_CONFIG_H

#include <map>
#include <string>

#include "../common_src/item_types.h"

struct GameConfig {
    int initial_health;
    int initial_energy;
    int sword_damage;
    int day_duration_seconds;

    std::map<std::string, int> buy_prices;
    std::map<std::string, int> sell_prices;

    std::map<ItemType, int> crop_waterings;
    std::map<ItemType, int> crop_growth_days;
    std::map<ItemType, int> crop_regrowth_days;
    std::map<ItemType, int> crop_missed_water_tolerance;

    int till_energy_cost;
    int plant_energy_cost;
    int water_energy_cost;
    int harvest_energy_cost;

    int quality_watering_minutes_per_point;
    int quality_missed_watering_penalty;
    int quality_harvest_minutes_per_point;

    int quality_luck_min_pct;
    int quality_luck_max_pct;

    int quality_good_threshold;
    int quality_excellent_threshold;
    int quality_incredible_threshold;
    int quality_perfect_threshold;

    static GameConfig load(const std::string& path);
};

#endif
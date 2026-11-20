
#ifndef GAME_CONFIG_H
#define GAME_CONFIG_H

#include <string>
#include <map>

struct GameConfig {
    int initial_health;
    int initial_energy;
    int sword_damage;
    int day_duration_seconds;

    std::map<std::string, int> buy_prices;
    std::map<std::string, int> sell_prices;

    static GameConfig load(const std::string& path);
};

#endif
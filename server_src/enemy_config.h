#ifndef ENEMY_CONFIG_H
#define ENEMY_CONFIG_H

#include <string>
#include <vector>

#include "../common_src/item_types.h"

struct EnemyDropRule {
    ItemType item_type;
    Quality quality;
    int min_quantity;
    int max_quantity;
    float probability;
};

struct EnemyTypeConfig {
    int max_health = 1;
    int damage = 1;
    float speed = 1.0f;
    float attack_cooldown = 1.0f;
    float attack_range = 1.0f;
    std::vector<EnemyDropRule> drops;
};

struct SkullConfig {
    float shot_cooldown = 1.0f;
    float shot_range = 1.0f;
    int blue_flame_damage = 1;
    float blue_flame_speed = 1.0f;
    int green_flame_damage = 1;
    float green_flame_speed = 1.0f;
};

struct EnemyConfig {
    EnemyTypeConfig skeleton;
    float skeleton_revive_seconds = 1.0f;
    EnemyTypeConfig orc;
    int orc_group_min = 1;
    int orc_group_max = 1;
    EnemyTypeConfig female_vampire;
    EnemyTypeConfig male_vampire;
    float blood_incident_radius = 1.0f;
    float blood_revive_seconds = 1.0f;
    float frenzy_multiplier = 1.0f;
    SkullConfig skull;
    int skeletons_min = 1;
    int skeletons_max = 1;
    int vampire_pairs_min = 1;
    int vampire_pairs_max = 1;
    int skulls_per_level = 1;

    static EnemyConfig load(const std::string& path);
};

#endif

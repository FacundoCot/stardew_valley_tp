#ifndef MELEE_H
#define MELEE_H

#include "enemy.h"

struct MeleeParams {
    int damage;
    float move_speed;
    float attack_cooldown;
    float attack_range;
};

bool chase_and_attack(EnemyWorld& world, ZoneId zone, float& x, float& y,
                      const MeleeParams& params, float& cooldown_left, float dt);

#endif

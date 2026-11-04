#ifndef SKULL_H
#define SKULL_H

#include "enemy.h"

class Skull : public Enemy {
private:
    EnemyStats stats;
    float shot_cooldown;

public:
    Skull(float x, float y, const EnemyStats& enemy_stats);

    bool act(EnemyWorld& world, float dt) override;
    EnemyKind kind() const override;

    void take_damage(int damage) override;

    bool is_dead() const override;
    bool should_be_removed() const override;
};

#endif
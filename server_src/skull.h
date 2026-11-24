#ifndef SKULL_H
#define SKULL_H

#include "enemy.h"
#include "enemy_config.h"

class Skull : public Enemy {
private:
    bool green;
    SkullConfig config;
    float shot_cooldown;

public:
    Skull(ZoneId zone, float x, float y, bool green, const SkullConfig& config);

    bool act(EnemyWorld& world, float dt) override;
    EnemyKind kind() const override;

    void take_damage(int damage) override;

    bool is_dead() const override;
    bool should_be_removed() const override;
};

#endif
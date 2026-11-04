#ifndef ORC_H
#define ORC_H

#include "enemy.h"

class Orc : public Enemy {
private:
    EnemyStats stats;
    float attack_cooldown;

    bool try_move(float new_x, float new_y, EnemyWorld& world);

public:
    Orc(float x, float y, const EnemyStats& enemy_stats);

    bool act(EnemyWorld& world, float dt) override;
    EnemyKind kind() const override;
};

#endif
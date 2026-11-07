#ifndef SKELETON_H
#define SKELETON_H

#include "enemy.h"

class Skeleton : public Enemy {
private:
    EnemyStats stats;
    float revive_time;     
    float revive_left;     
    bool waiting_revive;
    float attack_cooldown;

    bool try_move(float new_x, float new_y, EnemyWorld& world);

public:
    Skeleton(float x, float y, const EnemyStats& enemy_stats, float revive_time);

    bool act(EnemyWorld& world, float dt) override;
    EnemyKind kind() const override;

    void take_damage(int damage) override;
    bool should_be_removed() const override;
    bool is_visible() const override;
};

#endif
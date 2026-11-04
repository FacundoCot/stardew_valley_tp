#ifndef VAMPIRE_H
#define VAMPIRE_H

#include "enemy.h"

class Vampire : public Enemy {
private:
    EnemyStats stats;
    bool female;
    int pair_id;
    bool frenzy;
    float attack_cooldown;

    bool try_move(float new_x, float new_y, EnemyWorld& world);

public:
    Vampire(float x, float y, const EnemyStats& enemy_stats,
            bool is_female, int pair_id);

    bool act(EnemyWorld& world, float dt) override;

    EnemyKind kind() const override;

    bool is_female() const;
    int get_pair_id() const;

    void enter_frenzy();
};

#endif
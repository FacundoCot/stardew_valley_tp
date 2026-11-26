#ifndef VAMPIRE_H
#define VAMPIRE_H

#include "enemy.h"

class Vampire : public Enemy {
private:
    EnemyStats stats;
    bool female;
    bool frenzy;
    float speed_multiplier;
    float damage_multiplier;
    float frenzy_multiplier;
    float attack_cooldown;

public:
    Vampire(ZoneId zone, float x, float y, const EnemyStats& enemy_stats,
            bool is_female);

    bool act(EnemyWorld& world, float dt) override;

    EnemyKind kind() const override;
    bool is_visible() const override;

    bool is_female() const;
    bool has_frenzy() const;
    void on_partner_died() override;
    void set_frenzy_multiplier(float multiplier);
};

#endif
#include "vampire.h"

#include "melee.h"

Vampire::Vampire(ZoneId zone, float x, float y, const EnemyStats& enemy_stats,
                 bool is_female):
    Enemy(zone, x, y, enemy_stats.max_health), stats(enemy_stats),
    female(is_female), frenzy(false), speed_multiplier(1.0f),
    damage_multiplier(1.0f), frenzy_multiplier(2.0f), attack_cooldown(0.0f) {}

bool Vampire::act(EnemyWorld& world, float dt) {
    if (is_waiting_revive()) {
        return tick_revive(dt);
    }
    if (is_dead()) return false;
    return chase_and_attack(world, zone, x, y,
                            {static_cast<int>(stats.damage * damage_multiplier),
                             stats.speed * speed_multiplier,
                             stats.attack_cooldown, stats.attack_range},
                            attack_cooldown, dt);
}

EnemyKind Vampire::kind() const { return EnemyKind::VAMPIRE; }
bool Vampire::is_visible() const { return !is_waiting_revive(); }
bool Vampire::is_female() const { return female; }
bool Vampire::has_frenzy() const { return frenzy; }
void Vampire::set_frenzy_multiplier(float multiplier) { frenzy_multiplier = multiplier; }

void Vampire::on_partner_died() {
    if (!is_dead() && !frenzy) {
        frenzy = true;
        if (female) speed_multiplier = frenzy_multiplier;
        else damage_multiplier = frenzy_multiplier;
    }
}

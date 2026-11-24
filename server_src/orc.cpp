#include "orc.h"

#include "melee.h"

Orc::Orc(ZoneId zone, float x, float y, const EnemyStats& enemy_stats):
    Enemy(zone, x, y, enemy_stats.max_health), stats(enemy_stats),
    attack_cooldown(0.0f) {}

bool Orc::act(EnemyWorld& world, float dt) {
    if (is_dead()) return false;
    return chase_and_attack(world, zone, x, y,
                            {stats.damage, stats.speed, stats.attack_cooldown,
                             stats.attack_range},
                            attack_cooldown, dt);
}

EnemyKind Orc::kind() const { return EnemyKind::ORC; }

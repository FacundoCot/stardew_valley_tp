#include "skeleton.h"

#include "melee.h"

Skeleton::Skeleton(ZoneId zone, float x, float y, const EnemyStats& enemy_stats,
                   float revive_time):
    Enemy(zone, x, y, enemy_stats.max_health),
    stats(enemy_stats),
    revive_time(revive_time),
    attack_cooldown(0.0f) {}

EnemyKind Skeleton::kind() const {
    return EnemyKind::SKELETON;
}

void Skeleton::take_damage(int damage) {
    if (is_waiting_revive()) {
        return;
    }
    Enemy::take_damage(damage);
    if (is_dead()) {
        // Revive en el lugar de la muerte con vida máxima (decisión a confirmar).
        start_revive(revive_time);
    }
}

bool Skeleton::should_be_removed() const {
    return false;
}

bool Skeleton::is_visible() const {
    return !is_waiting_revive();
}

bool Skeleton::act(EnemyWorld& world, float dt) {
    if (is_waiting_revive()) {
        return tick_revive(dt);
    }
    if (is_dead()) {
        return false;
    }
    return chase_and_attack(world, zone, x, y,
                            {stats.damage, stats.speed, stats.attack_cooldown,
                             stats.attack_range},
                            attack_cooldown, dt);
}
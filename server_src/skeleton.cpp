#include "skeleton.h"

#include <algorithm>
#include <cmath>

namespace {
    float sign_step(float distance, float step) {
        if (distance > 0.0f) {
            return std::min(distance, step);
        } else if (distance < 0.0f) {
            return std::max(distance, -step);
        }
        return 0.0f;
    }
}

Skeleton::Skeleton(float x, float y, const EnemyStats& enemy_stats, float revive_time_param):
    Enemy(x, y, enemy_stats.max_health),
    stats(enemy_stats),
    revive_time(revive_time_param),
    revive_left(0.0f),
    waiting_revive(false),
    attack_cooldown(0.0f) {}

EnemyKind Skeleton::kind() const {
    return EnemyKind::SKELETON;
}

bool Skeleton::try_move(float new_x, float new_y, EnemyWorld& world) {
    if (world.can_walk(new_x, new_y)) {
        x = new_x;
        y = new_y;
        return true;
    }
    return false;
}

void Skeleton::take_damage(int damage) {
    if (waiting_revive) {
        return;
    }
    Enemy::take_damage(damage);
    if (is_dead()) {
        waiting_revive = true;
        revive_left = revive_time;
    }
}

bool Skeleton::should_be_removed() const {
    return is_dead() && !waiting_revive;
}

bool Skeleton::is_visible() const {
    return !waiting_revive;
}

bool Skeleton::act(EnemyWorld& world, float dt) {
    if (waiting_revive) {
        revive_left -= dt;
        if (revive_left > 0.0f) {
            return false;
        }
        // Revive en el lugar de la muerte con vida máxima (decisión a confirmar).
        waiting_revive = false;
        health = max_health;
        attack_cooldown = 0.0f;
        return true;
    }

    attack_cooldown = std::max(0.0f, attack_cooldown - dt);

    EnemyWorld::Target target;
    if (!world.nearest_player(x, y, target)) {
        return false;
    }

    float dist_x = target.x - x;
    float dist_y = target.y - y;

    if (std::max(std::abs(dist_x), std::abs(dist_y)) <= stats.attack_range) {
        if (attack_cooldown > 0.0f) {
            return false;
        }
        attack_cooldown = stats.attack_cooldown;
        return world.damage_player(target.name, stats.damage);
    }

    float step = stats.speed * dt;
    float step_x = sign_step(dist_x, step);
    float step_y = sign_step(dist_y, step);

    bool moved = false;
    if (std::abs(dist_x) >= std::abs(dist_y)) {
        moved = (step_x != 0.0f && try_move(x + step_x, y, world))
             || (step_y != 0.0f && try_move(x, y + step_y, world));
    } else {
        moved = (step_y != 0.0f && try_move(x, y + step_y, world))
             || (step_x != 0.0f && try_move(x + step_x, y, world));
    }
    return moved;
}
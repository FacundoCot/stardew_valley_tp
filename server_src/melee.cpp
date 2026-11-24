#include "melee.h"

#include <algorithm>
#include <cmath>

bool chase_and_attack(EnemyWorld& world, ZoneId zone, float& x, float& y,
                      const MeleeParams& params, float& cooldown_left, float dt) {
    cooldown_left = std::max(0.0f, cooldown_left - dt);
    EnemyWorld::Target target;
    if (!world.nearest_player(zone, x, y, target)) {
        return false;
    }
    const float dx = target.x - x;
    const float dy = target.y - y;
    if (std::max(std::abs(dx), std::abs(dy)) <= params.attack_range) {
        if (cooldown_left > 0.0f) return false;
        cooldown_left = params.attack_cooldown;
        return world.damage_player(target.name, params.damage);
    }
    const float step = params.move_speed * dt;
    const float sx = dx > 0 ? std::min(dx, step) : std::max(dx, -step);
    const float sy = dy > 0 ? std::min(dy, step) : std::max(dy, -step);
    bool moved = false;
    if (std::abs(dx) >= std::abs(dy)) {
        if (sx != 0 && world.can_walk(zone, x + sx, y)) {
            x += sx; moved = true;
        } else if (sy != 0 && world.can_walk(zone, x, y + sy)) {
            y += sy; moved = true;
        }
    } else {
        if (sy != 0 && world.can_walk(zone, x, y + sy)) {
            y += sy; moved = true;
        } else if (sx != 0 && world.can_walk(zone, x + sx, y)) {
            x += sx; moved = true;
        }
    }
    return moved;
}

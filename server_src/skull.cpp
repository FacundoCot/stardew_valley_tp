#include "skull.h"

#include <algorithm>
#include <cmath>

Skull::Skull(ZoneId zone, float x, float y, bool green_param,
             const SkullConfig& config_param):
    Enemy(zone, x, y, 1), green(green_param), config(config_param),
    shot_cooldown(0.0f) {}

bool Skull::act(EnemyWorld& world, float dt) {
    shot_cooldown = std::max(0.0f, shot_cooldown - dt);
    EnemyWorld::Target target;
    if (!world.nearest_player(zone, x, y, target)) return false;
    if (std::max(std::abs(target.x - x), std::abs(target.y - y)) > config.shot_range ||
        shot_cooldown > 0.0f) return false;
    const float distance = std::hypot(target.x - x, target.y - y);
    if (distance == 0.0f) return false;
    const int damage = green ? config.green_flame_damage : config.blue_flame_damage;
    const float speed = green ? config.green_flame_speed : config.blue_flame_speed;
    world.spawn_projectile({zone, x, y, (target.x - x) / distance,
                            (target.y - y) / distance, speed, damage,
                            config.shot_range, green});
    shot_cooldown = config.shot_cooldown;
    return true;
}

EnemyKind Skull::kind() const { return EnemyKind::SKULL; }
void Skull::take_damage(int) {}
bool Skull::is_dead() const { return false; }
bool Skull::should_be_removed() const { return false; }

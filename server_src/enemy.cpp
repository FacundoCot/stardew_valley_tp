#include "enemy.h"

Enemy::Enemy(ZoneId zone_param, float start_x, float start_y, int max_hp):
    x(start_x), y(start_y), health(max_hp), max_health(max_hp), zone(zone_param) {}

float Enemy::get_x() const { return x; }
float Enemy::get_y() const { return y; }
int Enemy::get_health() const { return health; }
int Enemy::get_max_health() const { return max_health; }
ZoneId Enemy::get_zone() const { return zone; }
int Enemy::get_group_id() const { return group_id; }
void Enemy::set_group_id(int id) { group_id = id; }

void Enemy::start_revive(float seconds) {
    waiting_revive = true;
    revive_left = seconds;
}

bool Enemy::tick_revive(float dt) {
    if (!waiting_revive) return false;
    revive_left -= dt;
    if (revive_left > 0.0f) return false;
    waiting_revive = false;
    health = max_health;
    return true;
}

bool Enemy::is_waiting_revive() const { return waiting_revive; }

void Enemy::take_damage(int damage) {
    health -= damage;
    if (health < 0) {
        health = 0;
    }
}

bool Enemy::is_dead() const {
    return health <= 0;
}
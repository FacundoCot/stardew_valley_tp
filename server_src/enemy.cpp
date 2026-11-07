#include "enemy.h"

Enemy::Enemy(float start_x, float start_y, int max_hp):
    x(start_x), y(start_y), health(max_hp), max_health(max_hp) {}

float Enemy::get_x() const { return x; }
float Enemy::get_y() const { return y; }
int Enemy::get_health() const { return health; }
int Enemy::get_max_health() const { return max_health; }

void Enemy::take_damage(int damage) {
    health -= damage;
    if (health < 0) {
        health = 0;
    }
}

bool Enemy::is_dead() const {
    return health <= 0;
}
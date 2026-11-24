#include "projectile.h"

#include <cmath>

bool Projectile::update(float dt) {
    const float distance = spec.speed * dt;
    spec.x += spec.dir_x * distance;
    spec.y += spec.dir_y * distance;
    travelled += distance;
    return travelled <= spec.max_range;
}

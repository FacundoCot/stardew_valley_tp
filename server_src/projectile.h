#ifndef PROJECTILE_H
#define PROJECTILE_H

#include "../common_src/zone.h"

struct ProjectileSpec {
    ZoneId zone;
    float x;
    float y;
    float dir_x;
    float dir_y;
    float speed;
    int damage;
    float max_range;
    bool green;
};

class Projectile {
private:
    ProjectileSpec spec;
    float travelled = 0.0f;

public:
    explicit Projectile(const ProjectileSpec& value): spec(value) {}
    bool update(float dt);
    const ProjectileSpec& get_spec() const { return spec; }
    float get_x() const { return spec.x; }
    float get_y() const { return spec.y; }
};

#endif

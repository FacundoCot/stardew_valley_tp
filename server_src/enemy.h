#ifndef ENEMY_H
#define ENEMY_H

#include <string>

#include "../common_src/messages.h"
#include "projectile.h"

class EnemyWorld {
public:
    struct Target {
        std::string name;
        float x;
        float y;
    };

    virtual bool can_walk(ZoneId zone, float x, float y) = 0;
    virtual bool nearest_player(ZoneId zone, float x, float y, Target& target) = 0;
    virtual bool damage_player(const std::string& player_name, int damage) = 0;
    virtual void spawn_projectile(const ProjectileSpec& projectile) = 0;
    virtual ~EnemyWorld() = default;
};

struct EnemyStats {
    int max_health;
    int damage;
    float speed;            
    float attack_cooldown;  
    float attack_range;     
};

class Enemy {
protected:
    float x;
    float y;
    int health;
    int max_health;
    ZoneId zone;
    int group_id = 0;
    float revive_left = 0.0f;
    bool waiting_revive = false;

public:
    Enemy(ZoneId zone, float start_x, float start_y, int max_hp);

    virtual bool act(EnemyWorld& world, float dt) = 0;
    virtual EnemyKind kind() const = 0;

    virtual void take_damage(int damage);

    virtual bool is_dead() const;

    virtual bool should_be_removed() const { return is_dead() && !waiting_revive; }

    virtual bool is_visible() const { return true; }

    virtual void on_partner_died() {}

    float get_x() const;
    float get_y() const;
    int get_health() const;
    int get_max_health() const;
    ZoneId get_zone() const;
    int get_group_id() const;
    void set_group_id(int id);
    void start_revive(float seconds);
    bool tick_revive(float dt);
    bool is_waiting_revive() const;

    virtual ~Enemy() = default;
};

#endif
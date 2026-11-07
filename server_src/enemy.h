#ifndef ENEMY_H
#define ENEMY_H

#include <string>

#include "../common_src/messages.h"

class EnemyWorld {
public:
    struct Target {
        std::string name;
        float x;
        float y;
    };

    virtual bool can_walk(float x, float y) = 0;
    virtual bool nearest_player(float x, float y, Target& target) = 0;
    virtual bool damage_player(const std::string& player_name, int damage) = 0;
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

public:
    Enemy(float start_x, float start_y, int max_hp);

    virtual bool act(EnemyWorld& world, float dt) = 0;
    virtual EnemyKind kind() const = 0;

    virtual void take_damage(int damage);

    virtual bool is_dead() const;

    virtual bool should_be_removed() const { return is_dead(); }

    virtual bool is_visible() const { return true; }

    
    virtual int collect_gold_drop() { return 0; }

    float get_x() const;
    float get_y() const;
    int get_health() const;
    int get_max_health() const;

    virtual ~Enemy() = default;
};

#endif
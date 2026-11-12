#ifndef GAME_H
#define GAME_H

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "common_src/map.h"
#include "common_src/messages.h"
#include "common_src/queue.h"
#include "client_action.h"
#include "enemy.h"
#include "storage.h"
#include "game_clock.h"
#include "save_data_structs.h"

const int MAX_HEALTH = 100;
const int WEAKENED_HEALTH_PERCENT = 20;
const int SWORD_DAMAGE = 25;
const float PLAYER_ATTACK_COOLDOWN = 0.5f;  // segundos

inline bool is_weakened(int health) {
    return health > 0 && health * 100 <= MAX_HEALTH * WEAKENED_HEALTH_PERCENT;
}

struct Player {
    float x;
    float y;
    int health;
    bool unconscious;
    Direction facing;
    float attack_cooldown_left;
    Storage inventory{9};
    int coins;

    Player(float x, float y, int health, int initial_coins = 0):
        x(x), y(y), health(health), unconscious(health == 0),
        facing(Direction::DOWN), attack_cooldown_left(0.0f),
        inventory(9), coins(initial_coins) {}
};

class Game {
private:
    class EnemyView;

    Map map;
    GameClock clock;
    std::vector<std::unique_ptr<Enemy>> enemies;
    std::map<std::string, Player> players;          // nombre -> jugador
    std::map<int, std::string> connected_clients;   // client_id -> nombre
    mutable std::mutex mtx;
    Queue<std::shared_ptr<ClientAction>> actions;

    // Versiones SIN lock: solo se llaman con mtx ya tomado
    bool damage_player_unlocked(const std::string& player_name, int amount);
    bool nearest_player_unlocked(float x, float y, EnemyWorld::Target& target) const;
    bool remove_dead_enemies_unlocked();
    uint32_t get_initial_coins() const;

public:
    explicit Game(Map map);

    bool add_player(int client_id, const std::string& player_name, float x, float y);
    bool add_player_auto(int client_id, const std::string& player_name);
    bool move_player(int client_id, Direction direction);
    bool remove_player(int client_id);
    bool is_empty() const;
    std::vector<int> player_ids() const;
    std::vector<PlayerState> snapshot() const;

    bool damage_player(const std::string& player_name, int amount);
    bool attack(int client_id);
    bool update(float dt);

    void add_enemy(std::unique_ptr<Enemy> enemy);
    std::vector<EnemyState> enemy_snapshot() const;

    Queue<std::shared_ptr<ClientAction>>& get_actions_queue();

    InventoryState inventory_state(int client_id) const;
    TimeState time_state() const;
    bool player_name(int client_id, std::string& player_name) const;
    bool name_to_client_id(int& client_id, const std::string& player_name) const;
    GameSaveData get_save_data(const std::string& game_code, const std::string& game_name, const std::string& owner_name, bool multiplayer) const;
    void restore_save_data(const GameSaveData& data);
};

#endif

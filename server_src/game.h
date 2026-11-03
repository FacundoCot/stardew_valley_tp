#ifndef GAME_H
#define GAME_H

#include <map>
#include <mutex>
#include <vector>
#include <string>

#include "common_src/map.h"
#include "common_src/messages.h"
#include "common_src/queue.h"      // falta
#include "client_action.h" 

const int MAX_HEALTH = 100;
const int WEAKENED_HEALTH_PERCENT = 20;

inline bool is_weakened(int health) {
    return health > 0 && health * 100 <= MAX_HEALTH * WEAKENED_HEALTH_PERCENT;
}


struct Player {
    float x;
    float y;
    int health;
    bool unconscious;

    Player(float x, float y, int health):
        x(x), y(y), health(health), unconscious(health == 0) {}
};

class Game {
private:
    Map map;
    std::map<std::string, Player> players; //  client_ id -> posicion 
    std::map<int, std::string> connected_clients;
    mutable std::mutex mtx;
    Queue<std::shared_ptr<ClientAction>> actions;


public:
    explicit Game(Map map);

    // Falso si el id ya existe, la celda es pared, esta fuera del mapa
    bool add_player(int client_id, const std::string& player_name, float x, float y);

    // Falso si el jugador no existe, la celda destino es pared
    bool move_player(int client_id, Direction direction);

    bool remove_player(int client_id);

    bool is_empty() const;

    std::vector<int> player_ids() const;

    std::vector<PlayerState> snapshot() const;

    // Ubica al jugador en el spaw
    bool add_player_auto(int client_id, const std::string& player_name);

    bool damage_player(const std::string& player_name, int amount);

    Queue<std::shared_ptr<ClientAction>>& get_actions_queue();
};

#endif
#ifndef GAME_H
#define GAME_H

#include <map>
#include <mutex>
#include <vector>

#include "common_src/map.h"
#include "common_src/messages.h"
#include "common_src/queue.h"      // falta
#include "client_action.h" 

struct Player {
    float x;
    float y;
};

class Game {
private:
    Map map;
    std::map<int, Player> players; //  client_ id -> posicion 
    mutable std::mutex mtx;
    Queue<std::shared_ptr<ClientAction>> actions;


public:
    explicit Game(Map map);

    // Falso si el id ya existe, la celda es pared, esta fuera del mapa
    bool add_player(int client_id, float x, float y);

    // Falso si el jugador no existe, la celda destino es pared
    bool move_player(int client_id, Direction direction);

    bool remove_player(int client_id);

    bool is_empty() const;

    std::vector<int> player_ids() const;

    std::vector<PlayerState> snapshot() const;

    // Ubica al jugador en el spaw
    bool add_player_auto(int client_id);

    Queue<std::shared_ptr<ClientAction>>& get_actions_queue();
};

#endif
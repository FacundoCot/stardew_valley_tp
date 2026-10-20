#ifndef GAME_MANAGER_H
#define GAME_MANAGER_H

#include <map>
#include <random>
#include <string>

#include "../common_src/map.h"
#include "../common_src/messages.h"

#include "client_registry.h"
#include "game.h"
#include "game_action_handler.h"

class GameManager : public GameActionHandler {
private:
    ClientRegistry& registry;
    Map world_map;                           // mapa base; cada Game recibe una copia
    std::map<std::string, Game> games;       // codigo -> partida
    std::map<int, std::string> client_game;  // client_id -> codigo de su partida
    std::mt19937 rng;

    std::string generate_code();
    void send_error(int client_id, const std::string& text);
    void send_state(const Game& game);

public:
    GameManager(ClientRegistry& registry, Map world_map);

    void create_game(int client_id,
                     const std::string& game_name,
                     const std::string& player_name) override;

    void join_game(int client_id,
                   const std::string& game_code,
                   const std::string& player_name) override;

    void move_player(int client_id, Direction direction) override;

    void disconnect_client(int client_id) override;
};

#endif
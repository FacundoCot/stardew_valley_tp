#include "game.h"
#include <utility>

Game::Game(Map map_param):
        map(std::move(map_param)) {}


bool Game::add_player(int client_id, int x, int y){
    if (players.count(client_id) > 0)
    {
        return false;
    }

    if (!map.is_walkable(x, y))
    {
        return false;
    }
    
    players[client_id] = Player{x, y};
    return true;
}

bool Game::move_player(int client_id, Direction direction) {
    auto player_iterator = players.find(client_id);
    if (player_iterator == players.end()) {
        return false;
    }

    int new_x = player_iterator->second.x;
    int new_y = player_iterator->second.y;

    switch (direction) {
        case Direction::UP:    new_y--; break;
        case Direction::DOWN:  new_y++; break;
        case Direction::LEFT:  new_x--; break;
        case Direction::RIGHT: new_x++; break;
    }

    if (!map.is_walkable(new_x, new_y)) {
        return false;
    }

    player_iterator->second.x = new_x;
    player_iterator->second.y = new_y;
    return true;
}


bool Game::remove_player(int client_id){
    return players.erase(client_id) > 0;
}

bool Game::is_empty() const{
    return players.empty();
}


std::vector<int> Game::player_ids() const{
    std::vector<int> ids;
    ids.reserve(players.size());

    for (const auto& [id, player] : players)
    {
        ids.push_back(id);
    }
    return ids;
}


std::vector<PlayerState> Game::snapshot() const {
    std::vector<PlayerState> state;
    state.reserve(players.size());
    for (const auto& [id, player] : players) {
        state.push_back(PlayerState{
            static_cast<uint32_t>(id),
            static_cast<uint16_t>(player.x),
            static_cast<uint16_t>(player.y),
        });
    }
    return state;
}

bool Game::add_player_auto(int client_id) {
    std::vector<std::pair<int, int>> spawns = map.spawns();
    if (spawns.empty()) {
        return false;
    }
    auto [x, y] = spawns.front();
    return add_player(client_id, x, y);
}
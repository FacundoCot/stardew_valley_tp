#include "game.h"
#include <string>
#include <utility>


namespace {
    const float PLAYER_STEP = 0.1f;
}


Game::Game(Map map_param):
        map(std::move(map_param)) {}


bool Game::add_player(int client_id, float x, float y){
    std::lock_guard<std::mutex> lock(mtx);

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
    std::lock_guard<std::mutex> lock(mtx);

    auto player_iterator = players.find(client_id);
    if (player_iterator == players.end()) {
        return false;
    }

    float new_x = player_iterator->second.x;
    float new_y = player_iterator->second.y;

    switch (direction) {
        case Direction::UP:    new_y -= PLAYER_STEP; break;
        case Direction::DOWN:  new_y += PLAYER_STEP; break;
        case Direction::LEFT:  new_x -= PLAYER_STEP; break;
        case Direction::RIGHT: new_x += PLAYER_STEP; break;
    }

    if (!map.is_walkable(new_x, new_y)) {
        return false;
    }

    player_iterator->second.x = new_x;
    player_iterator->second.y = new_y;
    return true;
}


bool Game::remove_player(int client_id){
    std::lock_guard<std::mutex> lock(mtx);
    return players.erase(client_id) > 0;
}

bool Game::is_empty() const{
    std::lock_guard<std::mutex> lock(mtx);
    
    return players.empty();
}


std::vector<int> Game::player_ids() const{
    std::lock_guard<std::mutex> lock(mtx);

    std::vector<int> ids;
    ids.reserve(players.size());

    for (const auto& [id, player] : players)
    {
        ids.push_back(id);
    }
    return ids;
}


std::vector<PlayerState> Game::snapshot() const {
    std::lock_guard<std::mutex> lock(mtx);

    std::vector<PlayerState> state;
    state.reserve(players.size());
    for (const auto& [id, player] : players) {
        state.push_back(PlayerState{
            std::to_string(id), // -> hay q cambiar al nombre cuando se tenga
            player.x,
            player.y,
        });
    }
    return state;
}

bool Game::add_player_auto(int client_id) {
    std::vector<std::pair<float, float>> spawns = map.spawns();
    if (spawns.empty()) {
        return false;
    }
    auto [x, y] = spawns.front();
    return add_player(client_id, x, y);
}

Queue<std::shared_ptr<ClientAction>>& Game::get_actions_queue() {
    return actions;
}
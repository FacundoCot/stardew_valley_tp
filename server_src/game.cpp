#include "game.h"
#include <memory>
#include <string>
#include <utility>

namespace {
    const float PLAYER_STEP = 5.1f;

    bool apply_health(Player& player, int health) {
        int new_life = health;
        if (new_life < 0) new_life = 0;
        if (new_life > MAX_HEALTH) new_life = MAX_HEALTH;

        if (new_life == player.health) {
            return false;
        }
        player.health = new_life;
        player.unconscious = (new_life == 0);
        return true;
    }
}

Game::Game(Map map_param) : map(std::move(map_param)) {}

bool Game::add_player(int client_id, const std::string &player_name, float x,
                      float y) {
  std::lock_guard<std::mutex> lock(mtx);

  if (connected_clients.count(client_id) > 0) {
    return false;
  }

  for (const auto &[id, name] : connected_clients) {
    if (name == player_name) {
      return false;
    }
  }

  if (players.count(player_name) == 0) {
      if (!map.is_walkable(x, y)) {
          return false;
      }

      players.emplace(player_name, Player(x, y, MAX_HEALTH));
  }

  connected_clients[client_id] = player_name;
  return true;
}

bool Game::move_player(int client_id, Direction direction) {
    std::lock_guard<std::mutex> lock(mtx);

    auto client_iterator = connected_clients.find(client_id);
    if (client_iterator == connected_clients.end()) {
        return false;
    }

    auto player_iterator = players.find(client_iterator->second);
    if (player_iterator == players.end()) {
        return false;
    }


    if (player_iterator->second.unconscious) {
        return false;
    }

    float new_x = player_iterator->second.x;
    float new_y = player_iterator->second.y;

    switch (direction) {
        case Direction::UP:    new_y -= PLAYER_STEP; break;
        case Direction::DOWN:  new_y += PLAYER_STEP; break;
        case Direction::LEFT:  new_x -= PLAYER_STEP; break;
        case Direction::RIGHT: new_x += PLAYER_STEP; break;
        case Direction::DEFAULT: return false;
    }

    if (!map.is_walkable(new_x, new_y)) {
        return false;
    }

    player_iterator->second.x = new_x;
    player_iterator->second.y = new_y;
    return true;
}

bool Game::remove_player(int client_id) {
  std::lock_guard<std::mutex> lock(mtx);
  return connected_clients.erase(client_id) > 0;
}

bool Game::is_empty() const {
  std::lock_guard<std::mutex> lock(mtx);

  return connected_clients.empty();
}

std::vector<int> Game::player_ids() const {
  std::lock_guard<std::mutex> lock(mtx);

  std::vector<int> ids;
  ids.reserve(connected_clients.size());

  for (const auto &[id, player_name] : connected_clients) {
    ids.push_back(id);
  }
  return ids;
}

std::vector<PlayerState> Game::snapshot() const {
  std::lock_guard<std::mutex> lock(mtx);

  std::vector<PlayerState> state;
  state.reserve(connected_clients.size());

  for (const auto &[id, name] : connected_clients) {
    const Player &player = players.at(name);

        state.push_back(PlayerState{
            name,
            player.x,
            player.y,
            player.health,
            MAX_HEALTH,
            player.unconscious,
        });
    }
    return state;
}

bool Game::add_player_auto(int client_id, const std::string &player_name) {
  std::vector<std::pair<float, float>> spawns = map.spawns();
  if (spawns.empty()) {
    return false;
  }
  auto [x, y] = spawns.front();
  return add_player(client_id, player_name, x, y);
}

Queue<std::shared_ptr<ClientAction>>& Game::get_actions_queue() {
    return actions;
}

bool Game::damage_player(const std::string& player_name, int amount) {
    if (amount <= 0) {
        return false;
    }

    std::lock_guard<std::mutex> lock(mtx);

    auto player_iterator = players.find(player_name);
    if (player_iterator == players.end()) {
        return false;
    }

    return apply_health(player_iterator->second, player_iterator->second.health - amount);
}

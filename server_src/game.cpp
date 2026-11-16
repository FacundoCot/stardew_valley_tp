#include "game.h"
#include <algorithm>
#include <cmath>
#include <memory>
#include <string>
#include <utility>
#include <cstdlib>
#include <ctime>
#include <stdexcept>

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

uint32_t Game::get_initial_coins() const {
    // para que cambie en cada ejecucion
    srand(time(nullptr));

    // Generar número aleatorio entre 0 y 100
    return static_cast<uint32_t>(rand() % 101);
}

Game::Game(std::shared_ptr<const World> world_param):
        world(std::move(world_param)), clock(1, 840) {
    if (world == nullptr) {
        throw std::invalid_argument("el mundo no puede ser nulo");
    }
    farm = std::dynamic_pointer_cast<const FarmZone>(
        world->zone(ZoneId::FARM_ZONE));
    if (farm == nullptr) {
        throw std::invalid_argument("el mundo tiene que tener granja");
    }
}

bool Game::add_player_unlocked(int client_id, const std::string &player_name,
                               ZoneId zone, float x, float y, const CharacterAppearance& appearance) {
  if (connected_clients.count(client_id) > 0) {
    return false;
  }

  for (const auto &[id, name] : connected_clients) {
    if (name == player_name) {
      return false;
    }
  }

  if (players.count(player_name) == 0) {
      if (!world->is_walkable(zone, x, y)) {
          return false;
      }
      
      Player player(x, y, MAX_HEALTH, get_initial_coins(), zone, appearance);

      player.inventory.add_item(Item(ItemType::PICKAXE, Quality::BASE), 1);
      player.inventory.add_item(Item(ItemType::WATERING_CAN, Quality::BASE), 1);
      player.inventory.add_item(Item(ItemType::SHOVEL, Quality::BASE), 1);
      player.inventory.add_item(Item(ItemType::SWORD, Quality::BASE), 1);

      players.emplace(player_name, player);
  }

  connected_clients[client_id] = player_name;
  return true;
}

bool Game::add_player_auto(int client_id, const std::string& player_name, const CharacterAppearance& appearance) {
  std::lock_guard<std::mutex> lock(mtx);

  int bed_index;
  const auto assigned_bed = bed_by_player.find(player_name);
  if (assigned_bed != bed_by_player.end()) {
      bed_index = assigned_bed->second;
  } else {
      bed_index = -1;
      for (int candidate = 0; candidate < farm->bed_count(); ++candidate) {
          bool occupied = false;
          for (const auto& [other_player, other_bed] : bed_by_player) {
              if (other_player != player_name && other_bed == candidate) {
                  occupied = true;
                  break;
              }
          }
          if (!occupied) {
              bed_index = candidate;
              break;
          }
      }
      if (bed_index < 0) {
          return false;
      }
  }

  const auto [x, y] = farm->bed_position(bed_index);
  if (!add_player_unlocked(client_id, player_name, ZoneId::FARM_ZONE, x, y, appearance)) {
      return false;
  }
  bed_by_player[player_name] = bed_index;
  return true;
}

bool Game::is_multiplayer() const {
    std::lock_guard<std::mutex> lock(mtx);
    return multiplayer_enabled;
}

bool Game::enable_multiplayer() {
    std::lock_guard<std::mutex> lock(mtx);
    if (multiplayer_enabled) return false;
    multiplayer_enabled = true;
    return true;
}

Game::MovementResult Game::move_player_unlocked(
        Player& player, float new_x, float new_y) {
    const float dx = new_x - player.x;
    const float dy = new_y - player.y;
    const float distance = std::max(std::fabs(dx), std::fabs(dy));
    const int steps = std::max(1, static_cast<int>(std::ceil(distance / 0.25f)));

    for (int i = 1; i <= steps; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(steps);
        const float step_x = player.x + dx * t;
        const float step_y = player.y + dy * t;
        if (!world->is_walkable(player.zone, step_x, step_y)) {
            return MovementResult::BLOCKED;
        }
        const auto portal = world->portal_at(player.zone, step_x, step_y);
        if (portal.has_value()) {
            player.zone = portal->destination;
            player.x = portal->arrival.first;
            player.y = portal->arrival.second;
            return MovementResult::TRANSPORTED;
        }
    }
    player.x = new_x;
    player.y = new_y;
    return MovementResult::MOVED;
}

bool Game::move_player(int client_id, Direction direction) {
    std::lock_guard<std::mutex> lock(mtx);
    //si el dia termino no hay que moverse
    if (clock.is_finished()) {
        return false;
    }
    auto client_iterator = connected_clients.find(client_id);
    if (client_iterator == connected_clients.end()) {
        return false;
    }

    auto player_iterator = players.find(client_iterator->second);
    if (player_iterator == players.end()) {
        return false;
    }

    Player& player = player_iterator->second;
    if (player.unconscious) {
        return false;
    }

    float new_x = player.x;
    float new_y = player.y;
    switch (direction) {
        case Direction::UP:      new_y -= PLAYER_STEP; break;
        case Direction::DOWN:    new_y += PLAYER_STEP; break;
        case Direction::LEFT:    new_x -= PLAYER_STEP; break;
        case Direction::RIGHT:   new_x += PLAYER_STEP; break;
        case Direction::DEFAULT: return false;
    }

    player.facing = direction;

    return move_player_unlocked(player, new_x, new_y) !=
           MovementResult::BLOCKED;
}

bool Game::remove_player(int client_id) {
  std::lock_guard<std::mutex> lock(mtx);
  ready_clients.erase(client_id);
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
            player.appearance
        });
    }
    return state;
}

Queue<std::shared_ptr<ClientAction>>& Game::get_actions_queue() {
    return actions;
}

std::shared_ptr<const World> Game::get_world() const {
    return world;
}


// --- Helpers sin lock (se llaman con mtx tomado) ---

bool Game::damage_player_unlocked(const std::string& player_name, int amount) {
    if (amount <= 0) {
        return false;
    }
    auto it = players.find(player_name);
    if (it == players.end() || it->second.unconscious) {
        return false;
    }
    return apply_health(it->second, it->second.health - amount);
}

bool Game::nearest_player_unlocked(float x, float y, EnemyWorld::Target& target) const {
    bool found = false;
    float best_distance = 0.0f;

    for (const auto& [client_id, name] : connected_clients) {
        const Player& player = players.at(name);
        if (player.unconscious) {
            continue;
        }
        // Distancia de Chebyshev: la misma que usa el esqueleto para atacar
        float distance = std::max(std::abs(player.x - x), std::abs(player.y - y));
        if (!found || distance < best_distance) {
            found = true;
            best_distance = distance;
            target = EnemyWorld::Target{name, player.x, player.y};
        }
    }
    return found;
}

bool Game::remove_dead_enemies_unlocked() {
    const size_t before = enemies.size();
    enemies.erase(
        std::remove_if(enemies.begin(), enemies.end(),
                       [](const std::unique_ptr<Enemy>& e) { return e->should_be_removed(); }),
        enemies.end());
    return enemies.size() != before;
}



class Game::EnemyView : public EnemyWorld {
public:
    explicit EnemyView(Game& g) : game(g) {}

    bool can_walk(float x, float y) override {
        // provisional: los enemigos tendrán zona propia
        return game.world->is_walkable(ZoneId::FARM_ZONE, x, y);
    }

    bool nearest_player(float x, float y, Target& target) override {
        return game.nearest_player_unlocked(x, y, target);
    }

    bool damage_player(const std::string& player_name, int damage) override {
        return game.damage_player_unlocked(player_name, damage);
    }

private:
    Game& game;
};


bool Game::damage_player(const std::string& player_name, int amount) {
    std::lock_guard<std::mutex> lock(mtx);
    return damage_player_unlocked(player_name, amount);
}

bool Game::update(float dt) {
    std::lock_guard<std::mutex> lock(mtx);

    if (connected_clients.empty() || clock.is_finished()) {
        return false;
    }

    clock.update(dt);

    //para que si el dia termino se se actualicen los enemigos
    if (clock.is_finished()) {
        return false;
    }

    for (auto& entry : players) {
        entry.second.attack_cooldown_left =
            std::max(0.0f, entry.second.attack_cooldown_left - dt);
    }
    /*
    EnemyView view(*this);
    bool changed = false;
    for (auto& enemy : enemies) {
        if (enemy->act(view, dt)) {
            changed = true;
        }
    }

    if (remove_dead_enemies_unlocked()) {
        changed = true;
    }*/
    return true;
}

bool Game::attack(int client_id) {
    std::lock_guard<std::mutex> lock(mtx);

    //si el dia termino no hay que moverse
    if (clock.is_finished()) {
        return false;
    }

    auto client_it = connected_clients.find(client_id);
    if (client_it == connected_clients.end()) {
        return false;
    }
    auto player_it = players.find(client_it->second);
    if (player_it == players.end()) {
        return false;
    }

    Player& player = player_it->second;
    if (player.unconscious || player.attack_cooldown_left > 0.0f) {
        return false;
    }

    float target_x = player.x;
    float target_y = player.y;
    switch (player.facing) {
        case Direction::UP:      target_y -= 1.0f; break;
        case Direction::DOWN:    target_y += 1.0f; break;
        case Direction::LEFT:    target_x -= 1.0f; break;
        case Direction::RIGHT:   target_x += 1.0f; break;
        case Direction::DEFAULT: return false;
    }

    player.attack_cooldown_left = PLAYER_ATTACK_COOLDOWN;

    const int damage = is_weakened(player.health) ? SWORD_DAMAGE / 2 : SWORD_DAMAGE;
    const int target_cell_x = static_cast<int>(std::floor(target_x));
    const int target_cell_y = static_cast<int>(std::floor(target_y));

    for (auto& enemy : enemies) {
        if (!enemy->is_visible()) {
            continue;
        }
        const int enemy_cell_x = static_cast<int>(std::floor(enemy->get_x()));
        const int enemy_cell_y = static_cast<int>(std::floor(enemy->get_y()));
        if (enemy_cell_x == target_cell_x && enemy_cell_y == target_cell_y) {
            enemy->take_damage(damage);
            break;
        }
    }

    remove_dead_enemies_unlocked();

    return true;
}

void Game::add_enemy(std::unique_ptr<Enemy> enemy) {
    std::lock_guard<std::mutex> lock(mtx);
    enemies.push_back(std::move(enemy));
}

std::vector<EnemyState> Game::enemy_snapshot() const {

    std::vector<EnemyState> state;
    state.reserve(enemies.size());
    for (const auto& enemy : enemies) {
        if (!enemy->is_visible()) {
            continue;
        }
        state.push_back(EnemyState{
            enemy->kind(),
            enemy->get_x(),
            enemy->get_y(),
            enemy->get_health(),
            enemy->get_max_health(),
        });
    }
    return state;
}

InventoryState Game::inventory_state(int client_id) const {

    const std::string& player_name = connected_clients.at(client_id);
    const Player& player = players.at(player_name);

    InventoryState state;
    state.coins = static_cast<uint32_t>(player.coins);
    state.selected_slot = static_cast<uint8_t>(player.selected_slot);

    const auto& stacks = player.inventory.get_stacks();

    for (size_t i = 0; i < stacks.size(); i++) {
        if (!stacks[i]) {
            continue;
        }

        const Stack& stack = *stacks[i];

        InventorySlotState slot;
        slot.index = static_cast<uint8_t>(i);
        slot.quality = stack.get_item().get_quality();
        slot.type = stack.get_item().get_type();
        slot.quantity = static_cast<uint8_t>(stack.get_quantity());

        state.slot_vector.push_back(slot);
    }
    return state;
}

TimeState Game::time_state() const {

    TimeState state;
    state.day = static_cast<uint16_t>(clock.get_day());
    state.hour = static_cast<uint8_t>(clock.get_hour());
    state.minute = static_cast<uint8_t>(clock.get_minute());
    state.finished = clock.is_finished();
    return state;
}

ServerMessage Game::game_state(int client_id) const {
    std::lock_guard<std::mutex> lock(mtx);

    ServerMessage msg{};
    msg.type = MessageType::GAME_STATE;

    msg.players = snapshot();
    msg.enemies = enemy_snapshot();
    msg.inventory = inventory_state(client_id);
    msg.time = time_state();

    return msg;
}

bool Game::confirm_next_day(int client_id) {
    std::lock_guard<std::mutex> lock(mtx);

    if (!clock.is_finished()) {
        return false;
    }

    if (connected_clients.find(client_id) == connected_clients.end()) {
        return false;
    }

    if (ready_clients.find(client_id) != ready_clients.end()) {
        return false;
    }
    ready_clients.insert(client_id);
    return true;
}

bool Game::all_players_ready_no_mutex() const {
    if (!clock.is_finished() || connected_clients.empty()) {
        return false;
    }

    return ready_clients.size() == connected_clients.size();
}

bool Game::all_players_ready() const {
    std::lock_guard<std::mutex> lock(mtx);
    return all_players_ready_no_mutex();
}

bool Game::start_next_day_game() {
    std::lock_guard<std::mutex> lock(mtx);
    if (!all_players_ready_no_mutex()) {
        return false;
    }
    clock.start_next_day();
    ready_clients.clear();
    return true;
}

bool Game::player_name(int client_id, std::string& player_name) const {
    std::lock_guard<std::mutex> lock(mtx);

    auto found = connected_clients.find(client_id);
    if (found == connected_clients.end()) {
        return false;
    }
    player_name = found->second;
    return true;
}

bool Game::name_to_client_id(int& client_id, const std::string& player_name) const {
    std::lock_guard<std::mutex> lock(mtx);

    for (const auto& [id, name] : connected_clients) {
        if (name == player_name) {
            client_id = id;
            return true;
        }
    }
    return false;
}

GameSaveData Game::get_save_data(const std::string& game_code, const std::string& game_name, 
    const std::string& owner_name, const std::string& world_name, bool multiplayer) const {
    std::lock_guard<std::mutex> lock(mtx);

    GameSaveData data;
    data.game_code = game_code;
    data.game_name = game_name;
    data.owner_name = owner_name;
    data.world_name = world_name;
    data.multiplayer = multiplayer;
    data.day = clock.get_day();

    for (const auto& [player_name, player] : players) {
        PlayerSaveData player_data;
        player_data.name = player_name;
        player_data.appearance = player.appearance;
        player_data.x = player.x;
        player_data.y = player.y;
        player_data.health = player.health;
        player_data.coins = player.coins;
        player_data.zone = player.zone;
        // si hay una mala asignacion de la cama se va a lanzar una excepcion gracias al .at()
        player_data.bed_index = bed_by_player.at(player_name);
        player_data.selected_slot = player.selected_slot;

        const auto& stacks = player.inventory.get_stacks();

        for (size_t i = 0; i < stacks.size(); i++) {
            if (!stacks[i]) {
                continue;
            }

            const Stack& stack = *stacks[i];

            ItemSaveData item_data;
            item_data.type = stack.get_item().get_type();
            item_data.quality = stack.get_item().get_quality();
            item_data.quantity = stack.get_quantity();
            item_data.index = static_cast<int>(i);

            player_data.inventory.push_back(item_data);
        }

        data.players.push_back(player_data);
    }

    return data;
}


void Game::restore_save_data(const GameSaveData& data) {
    std::lock_guard<std::mutex> lock(mtx);

    players.clear();
    connected_clients.clear();
    ready_clients.clear();
    bed_by_player.clear();

    clock = GameClock(data.day, 840);
    multiplayer_enabled = data.multiplayer;

    for (const PlayerSaveData& saved_player : data.players) {
        Player player(
            saved_player.x,
            saved_player.y,
            saved_player.health,
            saved_player.coins,
            saved_player.zone,
            saved_player.appearance
        );
        player.selected_slot = saved_player.selected_slot;

        for (const ItemSaveData& saved_item : saved_player.inventory) {
            Item item(saved_item.type, saved_item.quality);

            if (!player.inventory.place_item(
                    saved_item.index, item, saved_item.quantity)) {
                throw std::runtime_error("Inventario guardado invalido");
            }
        }

        bed_by_player[saved_player.name] = saved_player.bed_index;


        players.emplace(saved_player.name, std::move(player));
    }
}

bool Game::select_slot(int client_id, int slot_index) {
    std::lock_guard<std::mutex> lock(mtx);

    auto client_it = connected_clients.find(client_id);
    if (client_it == connected_clients.end()) {
        return false;
    }

    Player& player = players.at(client_it->second);
    const auto& slots = player.inventory.get_stacks();

    if (slot_index < 0 ||
        static_cast<size_t>(slot_index) >= slots.size()) {
        return false;
    }

    if (player.selected_slot == slot_index) {
        return false;
    }

    player.selected_slot = slot_index;
    return true;
}

int Game::get_selected_slot(int client_id) const {
    std::lock_guard<std::mutex> lock(mtx);

    auto client_it = connected_clients.find(client_id);
    if (client_it == connected_clients.end()) {
        return -1;
    }

    return players.at(client_it->second).selected_slot;
}

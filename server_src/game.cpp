#include "game.h"
#include <algorithm>
#include <cmath>
#include <memory>
#include <string>
#include <utility>
#include <cstdlib>
#include <ctime>
#include <stdexcept>
#include <sstream>

#include "common_src/village_zone.h"

namespace {
    const float PLAYER_STEP = 5.1f;

    bool apply_health(Player& player, int health, int max_health) {
        int new_life = health;
        if (new_life < 0) new_life = 0;
        if (new_life > max_health) new_life = max_health;

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

Game::Game(std::shared_ptr<const World> world_param,
           const GameConfig& config_param):
        world(std::move(world_param)),
        config(config_param),
        item_catalog(config),
        clock(1, config.day_duration_seconds) {
    if (world == nullptr) {
        throw std::invalid_argument("el mundo no puede ser nulo");
    }
    farm = std::dynamic_pointer_cast<const FarmZone>(
        world->zone(ZoneId::FARM_ZONE));
    if (farm == nullptr) {
        throw std::invalid_argument("el mundo tiene que tener granja");
    }

    //inicializo los comerciantes
    merchants.emplace(MerchantType::PIERRE, std::make_unique<Pierre>(item_catalog));
    merchants.emplace(MerchantType::CLINT, std::make_unique<Clint>(item_catalog));
    merchants.emplace(MerchantType::ROBIN, std::make_unique<Robin>(item_catalog));
    merchants.emplace(MerchantType::MARNIE, std::make_unique<Marnie>(item_catalog));
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
        
        Player player(x, y, config.initial_health, config.initial_energy, get_initial_coins(), zone, appearance);

        player.inventory.add_item(Item(ItemType::SWORD, Quality::BASE), 1);
        player.inventory.add_item(Item(ItemType::PICKAXE, Quality::BASE), 1);
        player.inventory.add_item(Item(ItemType::SHOVEL, Quality::BASE), 1);
        player.inventory.add_item(Item(ItemType::WATERING_CAN, Quality::BASE), 1);
        
        
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

    float step = PLAYER_STEP;
    if (player.energy * 10 <= config.initial_energy) {
        step = PLAYER_STEP/ 2.0f;
    }

    float new_x = player.x;
    float new_y = player.y;
    switch (direction) {
        case Direction::UP:      new_y -= step; break;
        case Direction::DOWN:    new_y += step; break;
        case Direction::LEFT:    new_x -= step; break;
        case Direction::RIGHT:   new_x += step; break;
        case Direction::DEFAULT: return false;
    }

    player.facing = direction;

    if (move_player_unlocked(player, new_x, new_y) == MovementResult::BLOCKED) {
        return false;
    }

    if (player.active_merchant && !near_merchant_unlocked(player, *player.active_merchant)) {
        player.active_merchant.reset();
    }

    check_day_end_unlocked();

    return true;
}

bool Game::remove_player(int client_id) {
    std::lock_guard<std::mutex> lock(mtx);
    //elimino de ready para el dia sig
    ready_clients.erase(client_id);

    // elimino su active_merchant
    auto client_it = connected_clients.find(client_id);
    if (client_it != connected_clients.end()) {
        auto player_it = players.find(client_it->second);

        if (player_it != players.end()) {
            player_it->second.active_merchant.reset();
        }
    }   

    const bool removed = connected_clients.erase(client_id) > 0;

    if (removed) {
        check_day_end_unlocked();
    }
    return removed;
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

std::vector<PlayerState> Game::snapshot(ZoneId zone, const std::string& self_name) const {
    std::vector<PlayerState> state;

    for (const auto &[id, name] : connected_clients) {
        const Player &player = players.at(name);

        if(name != self_name && player.zone == zone) {
            state.push_back(PlayerState{
                    name,
                    player.x,
                    player.y,
                    player.health,
                    config.initial_health,
                    player.energy,
                    config.initial_energy,
                    player.unconscious, 
                    player.appearance
            });
        }
    }
    return state;
}

std::vector<MapPlayerState> Game::map_snapshot() const {
    std::vector<MapPlayerState> state;

    for (const auto& [id, name] : connected_clients) {
        const Player& player = players.at(name);

        state.push_back(MapPlayerState{
            name,
            player.x,
            player.y,
            player.zone
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
    const bool changed = apply_health(it->second, it->second.health - amount, config.initial_health);

    check_day_end_unlocked();

    return changed;
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

    check_day_end_unlocked();

    if (clock.is_finished()) {
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

    const int damage = is_weakened(player.health, config.initial_health) ? 
                            config.sword_damage / 2 : config.sword_damage;
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

    player.energy = std::max(0, player.energy - ATTACK_ENERGY_COST);

    if (player.energy == 0) {
        player.unconscious = true;
    }

    check_day_end_unlocked();

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

    const std::string& name = connected_clients.at(client_id);
    const Player& me = players.at(name);

    ServerMessage msg{};
    msg.type = MessageType::GAME_STATE;

    msg.zone = me.zone;

    msg.self_player = PlayerState{
        name,
        me.x,
        me.y,
        me.health,
        config.initial_health,
        me.energy,
        config.initial_energy,
        me.unconscious,
        me.appearance
    };

    msg.players = snapshot(me.zone, name);
    msg.map_players = map_snapshot();
    
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

    for (auto& [name, player] : players) {
        const bool slept_well = !player.unconscious && is_in_own_bed_unlocked(name, player);

        const auto [bed_x, bed_y] = farm->bed_position(bed_by_player.at(name));

        player.x = bed_x;
        player.y = bed_y;
        player.zone = ZoneId::FARM_ZONE;

        player.health = slept_well ? config.initial_health : config.initial_health / 2;

        player.energy = slept_well ? config.initial_energy : config.initial_energy / 2;

        player.unconscious = false;
        player.facing = Direction::DOWN;
        player.attack_cooldown_left = 0.0f;

        player.active_merchant.reset();
    }

    clock.start_next_day();
    ready_clients.clear();

    return true;
}

std::optional<CharacterAppearance> Game::appearance_for(const std::string& player_name) const {

    std::lock_guard<std::mutex> lock(mtx);

    const auto it = players.find(player_name);

    if (it == players.end()) {
        return std::nullopt;
    }

    return it->second.appearance;
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
        player_data.energy = player.energy;
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

    clock = GameClock(data.day, config.day_duration_seconds);
    multiplayer_enabled = data.multiplayer;

    for (const PlayerSaveData& saved_player : data.players) {
        Player player(
            saved_player.x,
            saved_player.y,
            saved_player.health,
            saved_player.energy,
            saved_player.coins,
            saved_player.zone,
            saved_player.appearance
        );
        // se hacen aparte porque no son parte del constructor de player
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

void Game::check_day_end_unlocked() {
    if (clock.is_finished() || connected_clients.empty()) {
        return;
    }

    for (const auto& connected : connected_clients) {
        const std::string& name = connected.second;

        if (!players.at(name).unconscious) {
            return;
        }
    }

    clock.finish_day();
}

bool Game::is_in_own_bed_unlocked(const std::string& name, const Player& player) const {

    if (!(player.zone == ZoneId::FARM_ZONE)) {
        return false;
    }

    const auto bed_searched = bed_by_player.find(name);

    if (bed_searched == bed_by_player.end()) {
        return false;
    }

    const auto [bed_x, bed_y] = farm->bed_position(bed_searched->second);

    return std::floor(player.x) == std::floor(bed_x) && std::floor(player.y) == std::floor(bed_y);
}

bool Game::buy_item(int client_id, MerchantType merchant, const std::string& product, int quantity) {
    std::lock_guard<std::mutex> lock(mtx);

    auto itemt = item_catalog.get_item_type(product);
    if (!itemt) {
        return false;
    }
    ItemType type = *itemt;

    if (quantity <= 0 || clock.is_finished()) {
        return false;
    }

    auto client_it = connected_clients.find(client_id);
    auto player_it = players.find(client_it->second);

    Player& player = player_it->second;

    if (player.active_merchant != merchant || !near_merchant_unlocked(player, merchant)) {
        return false;
    }

    if (player.unconscious) {
        return false;
    }

    auto merchant_it = merchants.find(merchant);

    if (merchant_it == merchants.end()) {
        return false;
    }

    auto price = merchant_it->second->get_buy_price(type);

    if (!price) {
        return false;
    }

    const int total = *price * quantity;

    if (total > player.coins) {
        return false;
    }

    Item item(type, Quality::BASE);

    if (!player.inventory.can_add_item(item, quantity)) {
        return false;
    }

    player.inventory.add_item(item, quantity);
    player.coins -= total;

    return true;
}

bool Game::sell_item(int client_id, MerchantType merchant, const std::string& product, int quantity) {
    std::lock_guard<std::mutex> lock(mtx);

    auto itemt = item_catalog.get_item_type(product);
    if (!itemt) {
        return false;
    }
    ItemType type = *itemt;

    if (quantity <= 0 || clock.is_finished()) {
        return false;
    }

    auto client_it = connected_clients.find(client_id);
    auto player_it = players.find(client_it->second);

    Player& player = player_it->second;

    if (player.active_merchant != merchant || !near_merchant_unlocked(player, merchant)) {
        return false;
    }

    auto merchant_it = merchants.find(merchant);

    if (merchant_it == merchants.end()) {
        return false;
    }

    if (player.unconscious || !merchant_it->second->buys(type)) {
        return false;
    }

    std::map<Quality, int> quantities_to_sell;
    int remaining = quantity;

    for (const auto& slot : player.inventory.get_stacks()) {
        if (!slot || slot->get_item().get_type() != type) {
            continue;
        }

        int quantity_to_sell = std::min(remaining, slot->get_quantity());
        Quality quality = slot->get_item().get_quality();

        quantities_to_sell[quality] += quantity_to_sell;
        remaining -= quantity_to_sell;

        if (remaining == 0) {
            break;
        }
    }

    if (remaining != 0) {
        return false;
    }

    int total = 0;

    for (const auto& [quality, amount] : quantities_to_sell) {
        auto price = merchant_it->second->get_sell_price(type, quality);

        if (!price) {
            return false;
        }

        total += *price * amount;
    }

    for (const auto& [quality, amount] : quantities_to_sell) {
        Item item(type, quality);

        if (!player.inventory.remove_quantity(item, amount)) {
            throw std::logic_error("Error al retirar los objetos vendidos");
        }
    }

    player.coins += total;

    return true;
}

bool Game::near_merchant_unlocked(const Player& player, MerchantType merchant) const {

    if (player.zone != ZoneId::VILLAGE_ZONE) {
        return false;
    }

    // tengo un puntero a zone, pero quiero uno a la clase hija village, se que existe
    // entonces hago un cast y convertimos de tipo zone a tipo village con 
    // static_pointer_cast
    auto village = std::static_pointer_cast<const VillageZone>(world->zone(ZoneId::VILLAGE_ZONE));

    for (const auto& position : village->merchants()) {
        if (position.type == merchant) {
            constexpr float MAX_DISTANCE = 6.0f;
            // nos devuelve la distancia entre el jugador y el comerciante
            float distance = std::hypot(player.x - position.x, 
                                        player.y - position.y);

            return distance <= MAX_DISTANCE;
        }
    }

    return false;
}

bool Game::interact_with_merchant(int client_id, MerchantType merchant) {
    std::lock_guard<std::mutex> lock(mtx);

    if (clock.is_finished() || merchants.find(merchant) == merchants.end()) {
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

    if (player.unconscious || !near_merchant_unlocked(player, merchant)) {
        return false;
    }

    player.active_merchant = merchant;

    return true;
}

std::optional<MerchantType> Game::get_active_merchant(int client_id) const {
    std::lock_guard<std::mutex> lock(mtx);

    if (clock.is_finished()) {
        return std::nullopt;
    }

    auto client_it = connected_clients.find(client_id);

    if (client_it == connected_clients.end()) {
        return std::nullopt;
    }

    auto player_it = players.find(client_it->second);

    if (player_it == players.end()) {
        return std::nullopt;
    }

    const Player& player = player_it->second;

    if (player.unconscious || !player.active_merchant) {
        return std::nullopt;
    }

    if (!near_merchant_unlocked(player, *player.active_merchant)) {
        return std::nullopt;
    }

    return player.active_merchant;
}

std::optional<std::string> Game::get_merchant_catalog(int client_id) const {
    std::lock_guard<std::mutex> lock(mtx);

    if (clock.is_finished()) {
        return std::nullopt;
    }

    auto client_it = connected_clients.find(client_id);
    if (client_it == connected_clients.end()) {
        return std::nullopt;
    }

    auto player_it = players.find(client_it->second);
    if (player_it == players.end()) {
        return std::nullopt;
    }

    const Player& player = player_it->second;

    if (player.unconscious || !player.active_merchant ||
        !near_merchant_unlocked(player, *player.active_merchant)) {
        return std::nullopt;
    }

    auto merchant_it = merchants.find(*player.active_merchant);
    if (merchant_it == merchants.end()) {
        return std::nullopt;
    }

    const Merchant& merchant = *merchant_it->second;

    std::ostringstream catalog;

    catalog << "Items for sale:\n";

    bool has_sales = false;

    for (ItemType type : merchant.get_items_for_sale()) {
        auto price = merchant.get_buy_price(type);

        if (price) {
            catalog << item_catalog.get_item_name(type) << ": " << *price << " coins\n";
            has_sales = true;
        }
    }

    if (!has_sales) {
        catalog << "None\n";
    }

    catalog << "Items accepted for sale (base quality prices):\n";

    bool has_purchases = false;

    for (ItemType type : merchant.get_items_to_buy()) {
        auto price = merchant.get_sell_price(type, Quality::BASE);

        if (price) {
            catalog << item_catalog.get_item_name(type) << ": " << *price << " coins\n";
            has_purchases = true;
        }
    }

    if (!has_purchases) {
        catalog << "None\n";
    }

    return catalog.str();
}
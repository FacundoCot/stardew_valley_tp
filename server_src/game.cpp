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
#include <random>

#include "common_src/village_zone.h"
#include "vampire.h"
#include <iostream>

namespace {
    const float PLAYER_STEP = 1.0f;

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
           const GameConfig& config_param,
           const EnemyConfig& enemy_config_param):
        world(std::move(world_param)),
        config(config_param),
        enemy_config(enemy_config_param),
        item_catalog(config),
        clock(1, config.day_duration_seconds),
        rng(std::random_device{}()),
        drop_table(enemy_config),
        spawner(rng, enemy_config) {
    if (world == nullptr) {
        throw std::invalid_argument("el mundo no puede ser nulo");
    }
    farm = std::dynamic_pointer_cast<const FarmZone>(
        world->zone(ZoneId::FARM_ZONE));
    if (farm == nullptr) {
        throw std::invalid_argument("el mundo tiene que tener granja");
    }
    const auto house = std::dynamic_pointer_cast<const InteriorZone>(
        world->zone(ZoneId::HOUSE_ZONE));
    if (house == nullptr) {
        throw std::invalid_argument("el mundo tiene que tener casa");
    }
    if (house->beds().size() != 1) {
        throw std::invalid_argument("la casa debe tener exactamente una cama");
    }
    const auto [bed_x, bed_y] = house->bed_position(0);
    shared_bed_position = {
        std::floor(bed_x) + 0.5f,
        std::floor(bed_y) + 0.5f
    };
    if (!house->is_walkable(shared_bed_position.first, shared_bed_position.second)) {
        throw std::invalid_argument("la cama debe estar sobre un tile caminable");
    }

    //inicializo los comerciantes
    merchants.emplace(MerchantType::PIERRE, std::make_unique<Pierre>(item_catalog));
    merchants.emplace(MerchantType::CLINT, std::make_unique<Clint>(item_catalog));
    merchants.emplace(MerchantType::ROBIN, std::make_unique<Robin>(item_catalog, clock.get_day()));
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

    return add_player_unlocked(client_id, player_name, ZoneId::HOUSE_ZONE,
                               shared_bed_position.first, shared_bed_position.second,
                               appearance);
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
        const std::string& name, Player& player, float new_x, float new_y) {
    const float dx = new_x - player.x;
    const float dy = new_y - player.y;
    const float distance = std::max(std::fabs(dx), std::fabs(dy));
    const int steps = std::max(1, static_cast<int>(std::ceil(distance / 0.25f)));

    for (int i = 1; i <= steps; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(steps);
        const float step_x = player.x + dx * t;
        const float step_y = player.y + dy * t;

        const auto portal = world->portal_at(player.zone, step_x, step_y);
        if (portal.has_value()) {
            const ZoneId old_zone = player.zone;
            if (old_zone.kind == ZoneId::Kind::CAVE_LEVEL) {
                fight_sets.on_player_left_room(name);
            }
            player.zone = portal->destination;
            player.x = portal->arrival.first;
            player.y = portal->arrival.second;
            if (portal->destination.kind == ZoneId::Kind::CAVE_LEVEL) {
                const ZoneId room = portal->destination;
                const bool occupied = std::any_of(
                    connected_clients.begin(), connected_clients.end(),
                    [&](const auto& client) {
                        if (client.second == name) {
                            return false;
                        }
                        const auto connected_player = players.find(client.second);
                        return connected_player != players.end() &&
                               connected_player->second.zone == room;
                    });
                if (!occupied) {
                    enemies.erase(std::remove_if(enemies.begin(), enemies.end(),
                        [&](const std::unique_ptr<Enemy>& enemy) {
                            return enemy->get_zone() == room;
                        }), enemies.end());
                    projectiles.erase(std::remove_if(projectiles.begin(), projectiles.end(),
                        [&](const std::unique_ptr<Projectile>& projectile) {
                            return projectile->get_spec().zone == room;
                        }), projectiles.end());
                    auto cave = std::dynamic_pointer_cast<const CaveLevel>(
                        world->zone(room));
                    if (cave) {
                        auto spawned = spawner.populate(room, *cave, *world);
                        for (auto& enemy : spawned) {
                            enemies.push_back(std::move(enemy));
                        }
                    }
                }
                const bool has_skull = std::any_of(enemies.begin(), enemies.end(),
                    [&](const std::unique_ptr<Enemy>& enemy) {
                        return enemy->get_zone() == room &&
                               enemy->kind() == EnemyKind::SKULL;
                    });
                fight_sets.on_player_entered_room(name, room, has_skull);
            }
            return MovementResult::TRANSPORTED;
        }

        if (!world->is_walkable(player.zone, step_x, step_y)) {
            return MovementResult::BLOCKED;
        }

        if (player.zone == ZoneId::FARM_ZONE) {
            const CellPos pos{
                static_cast<int>(step_x), static_cast<int>(step_y)
            };

            if (has_placed_object_unlocked(pos)) {
                return MovementResult::BLOCKED;
            }
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

    if (move_player_unlocked(client_iterator->second, player, new_x, new_y) ==
        MovementResult::BLOCKED) {
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
        fight_sets.on_player_left_room(client_it->second);
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
                    player.appearance,
                    player.facing
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
    const bool changed = apply_health(it->second, it->second.health - amount,
                                      config.initial_health);
    if (changed) fight_sets.on_player_hurt_by_enemy(player_name);
    check_day_end_unlocked();

    return changed;
}

bool Game::nearest_player_unlocked(ZoneId zone, float x, float y, EnemyWorld::Target& target) const {
    bool found = false;
    float best_distance = 0.0f;

    for (const auto& [client_id, name] : connected_clients) {
        const Player& player = players.at(name);
        if (player.unconscious || player.zone != zone) {
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
    explicit EnemyView(Game& g): game(g) {}

    bool can_walk(ZoneId requested_zone, float x, float y) override {
        return game.world->is_walkable(requested_zone, x, y);
    }

    bool nearest_player(ZoneId requested_zone, float x, float y, Target& target) override {
        return game.nearest_player_unlocked(requested_zone, x, y, target);
    }

    bool damage_player(const std::string& player_name, int damage) override {
        return game.damage_player_unlocked(player_name, damage);
    }

    void spawn_projectile(const ProjectileSpec& projectile) override {
        game.projectiles.push_back(std::make_unique<Projectile>(projectile));
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
    check_day_end_unlocked();

    //para que si el dia termino se se actualicen los enemigos
    if (clock.is_finished()) {
        return false;
    }

    for (auto& entry : players) {
        entry.second.attack_cooldown_left =
            std::max(0.0f, entry.second.attack_cooldown_left - dt);
    }
    bool changed = false;
    for (auto& enemy : enemies) {
        EnemyView view(*this);
        if (enemy->act(view, dt)) {
            changed = true;
        }
    }

    projectiles.erase(std::remove_if(projectiles.begin(), projectiles.end(),
        [&](const std::unique_ptr<Projectile>& projectile) {
            changed = true;
            if (!projectile->update(dt)) {
                return true;
            }
            const auto& spec = projectile->get_spec();
            if (!world->is_walkable(spec.zone, projectile->get_x(), projectile->get_y())) {
                return true;
            }
            for (const auto& [name, player] : players) {
                if (player.zone == spec.zone && !player.unconscious &&
                    std::max(std::abs(player.x - projectile->get_x()),
                             std::abs(player.y - projectile->get_y())) < 0.5f) {
                    damage_player_unlocked(name, spec.damage);
                    return true;
                }
            }
            return false;
        }), projectiles.end());
    if (remove_dead_enemies_unlocked()) {
        changed = true;
    }
    return changed;
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
        if (!enemy->is_visible() || enemy->get_zone() != player.zone) {
            continue;
        }
        const int enemy_cell_x = static_cast<int>(std::floor(enemy->get_x()));
        const int enemy_cell_y = static_cast<int>(std::floor(enemy->get_y()));
        if (enemy_cell_x == target_cell_x && enemy_cell_y == target_cell_y) {
            const bool was_alive = !enemy->is_dead();
            enemy->take_damage(damage);
            if (was_alive && enemy->is_dead()) {
                handle_enemy_killed_unlocked(*enemy, client_it->second);
            }
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

void Game::handle_enemy_killed_unlocked(Enemy& enemy, const std::string& killer) {
    auto player = players.find(killer);
    if (player != players.end()) {
        for (const auto& [item, quantity] : drop_table.roll(enemy, rng)) {
            player->second.inventory.add_item(item, quantity);
        }
    }
    const int level = enemy.get_zone().kind == ZoneId::Kind::CAVE_LEVEL ?
                      enemy.get_zone().level : 0;
    fight_sets.on_enemy_killed(enemy, level);
    if (enemy.kind() != EnemyKind::VAMPIRE) return;
    for (auto& other : enemies) {
        if (other.get() != &enemy && other->kind() == EnemyKind::VAMPIRE &&
            other->get_group_id() == enemy.get_group_id()) {
            other->on_partner_died();
        }
    }
    if (player_incident_near_unlocked(enemy.get_zone(), enemy.get_x(), enemy.get_y(),
                                      enemy_config.blood_incident_radius)) {
        enemy.start_revive(enemy_config.blood_revive_seconds);
    }
}

bool Game::player_incident_near_unlocked(ZoneId zone, float x, float y,
                                          float radius) const {
    for (const auto& [name, player] : players) {
        if (player.zone == zone &&
            std::max(std::abs(player.x - x), std::abs(player.y - y)) <= radius &&
            (player.unconscious || is_weakened(player.health, config.initial_health))) {
            return true;
        }
    }
    return false;
}

std::vector<EnemyState> Game::enemy_snapshot(ZoneId zone) const {

    std::vector<EnemyState> state;
    state.reserve(enemies.size());
    for (const auto& enemy : enemies) {
        if (enemy->get_zone() != zone || !enemy->is_visible()) {
            continue;
        }
        EnemyVariant variant = EnemyVariant::NONE;
        if (enemy->kind() == EnemyKind::VAMPIRE) {
            const auto* vampire = dynamic_cast<const Vampire*>(enemy.get());
            variant = vampire->is_female() ? EnemyVariant::FEMALE_VAMPIRE :
                                             EnemyVariant::MALE_VAMPIRE;
        }
        state.push_back(EnemyState{
            enemy->kind(),
            enemy->get_x(),
            enemy->get_y(),
            enemy->get_health(),
            enemy->get_max_health(),
            variant,
            dynamic_cast<const Vampire*>(enemy.get()) != nullptr &&
                dynamic_cast<const Vampire*>(enemy.get())->has_frenzy()
        });
    }
    return state;
}

std::vector<ProjectileState> Game::projectile_snapshot(ZoneId zone) const {
    std::vector<ProjectileState> state;
    for (const auto& projectile : projectiles) {
        const auto& spec = projectile->get_spec();
        if (spec.zone == zone) {
            state.push_back({spec.green ? EnemyVariant::GREEN_FLAME :
                                         EnemyVariant::BLUE_FLAME,
                             projectile->get_x(), projectile->get_y()});
        }
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
        me.appearance,
        me.facing
    };

    msg.players = snapshot(me.zone, name);
    msg.map_players = map_snapshot();
    
    msg.enemies = enemy_snapshot(me.zone);
    msg.projectiles = projectile_snapshot(me.zone);
    msg.inventory = inventory_state(client_id);
    msg.time = time_state();
    msg.farm_plots = farm_plots_state();
    if (me.zone == ZoneId::FARM_ZONE) {
        msg.placed_objects = placed_objects_state();
    }
    msg.multiplayer_enabled = multiplayer_enabled;

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
        const bool slept_well = !player.unconscious && is_at_shared_bed_unlocked(player);

        player.x = shared_bed_position.first;
        player.y = shared_bed_position.second;
        player.zone = ZoneId::HOUSE_ZONE;

        player.health = slept_well ? config.initial_health : config.initial_health / 2;

        player.energy = slept_well ? config.initial_energy : config.initial_energy / 2;

        player.unconscious = false;
        player.facing = Direction::DOWN;
        player.attack_cooldown_left = 0.0f;

        player.active_merchant.reset();
    }

    for (auto& entry : farm_plots) {
        entry.second.advance_day();
    }

    clock.start_next_day();
    update_robin_catalog_unlocked();
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
        player_data.bed_index = 0;
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

    for (const auto& [pos, plot] : farm_plots) {
        FarmPlotSaveData plot_data{};

        plot_data.x = pos.x;
        plot_data.y = pos.y;

        const Crop* crop = plot.get_crop();

        if (crop) {
            plot_data.has_crop = true;
            plot_data.crop = crop->get_save_data();
        }

        data.farm_plots.push_back(plot_data);
    }

    for (const auto& [pos, type] : decorations) {
        data.decorations.push_back({
            pos.x,
            pos.y,
            type
        });
    }

    for (const auto& [pos, chest] : chests) {
        ChestSaveData chest_data{};

        chest_data.x = pos.x;
        chest_data.y = pos.y;

        const auto& stacks = chest.storage.get_stacks();

        for (size_t i = 0; i < stacks.size(); ++i) {
            if (!stacks[i]) {
                continue;
            }

            const Stack& stack = *stacks[i];

            chest_data.inventory.push_back({
                stack.get_item().get_type(),
                stack.get_item().get_quality(),
                stack.get_quantity(),
                static_cast<int>(i)
            });
        }

        data.chests.push_back(std::move(chest_data));
    }

        return data;
    }


void Game::restore_save_data(const GameSaveData& data) {
    std::lock_guard<std::mutex> lock(mtx);

    players.clear();
    connected_clients.clear();
    ready_clients.clear();
    farm_plots.clear();
    decorations.clear();
    chests.clear();

    clock = GameClock(data.day, config.day_duration_seconds);
    update_robin_catalog_unlocked();
    multiplayer_enabled = data.multiplayer;

    for (const PlayerSaveData& saved_player : data.players) {
        float x = saved_player.x;
        float y = saved_player.y;
        ZoneId zone = saved_player.zone;
        if (!world->is_walkable(zone, x, y)) {
            x = shared_bed_position.first;
            y = shared_bed_position.second;
            zone = ZoneId::HOUSE_ZONE;
        }
        Player player(
            x,
            y,
            saved_player.health,
            saved_player.energy,
            saved_player.coins,
            zone,
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

        players.emplace(saved_player.name, std::move(player));
    }

    for (const FarmPlotSaveData& saved_plot : data.farm_plots) {
        FarmPlot plot;

        if (saved_plot.has_crop) {
            plot.restore_crop(saved_plot.crop);
        }

        farm_plots.emplace(CellPos{saved_plot.x, saved_plot.y}, plot);
    }

    for (const auto& saved : data.decorations) {
        if (!is_decoration(saved.type) || !can_place_object_unlocked(saved.x, saved.y)) {
            throw std::runtime_error("Decoracion guardada invalida");
        }
        decorations.emplace(CellPos{saved.x, saved.y}, saved.type);
    }

    for (const auto& saved : data.chests) {
        if (!can_place_object_unlocked(saved.x, saved.y)) {
            throw std::runtime_error("Posicion de cofre invalida");
        }

        Chest chest;

        for (const auto& item_data : saved.inventory) {
            Item item(item_data.type, item_data.quality);

            if (!chest.storage.place_item(
                    item_data.index,
                    item,
                    item_data.quantity)) {
                throw std::runtime_error("Inventario de cofre invalido");
            }
        }

        chests.emplace(CellPos{saved.x, saved.y}, std::move(chest));
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
    close_unavailable_chests_unlocked();
    if (clock.is_finished()) {
        enemies.clear();
        projectiles.clear();
        return;
    }
    if (connected_clients.empty()) {
        return;
    }

    for (const auto& connected : connected_clients) {
        const std::string& name = connected.second;

        if (!players.at(name).unconscious) {
            return;
        }
    }

    clock.finish_day();
    close_unavailable_chests_unlocked();
    for (const auto& entry : players) {
        fight_sets.on_player_left_room(entry.first);
    }
    enemies.clear();
    projectiles.clear();
}

bool Game::is_at_shared_bed_unlocked(const Player& player) const {
    if (player.zone != ZoneId::HOUSE_ZONE) {
        return false;
    }

    return std::floor(player.x) == std::floor(shared_bed_position.first) &&
           std::floor(player.y) == std::floor(shared_bed_position.second);
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

    if (client_it == connected_clients.end()) {
        return false;
    }

    auto player_it = players.find(client_it->second);

    if (player_it == players.end()) {
        return false;
    }

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

    if (client_it == connected_clients.end()) {
        return false;
    }

    auto player_it = players.find(client_it->second);

    if (player_it == players.end()) {
        return false;
    }

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

void Game::update_robin_catalog_unlocked() {
    merchants.at(MerchantType::ROBIN) =
        std::make_unique<Robin>(item_catalog, clock.get_day());
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


bool Game::till_soil(int client_id, int x, int y) {
    std::lock_guard<std::mutex> lock(mtx);

    if (clock.is_finished()) {
        return false;
    }

    auto client_it = connected_clients.find(client_id);

    if (client_it == connected_clients.end()) {
        return false;
    }

    Player& player = players.at(client_it->second);

    if (player.zone != ZoneId::FARM_ZONE 
        || player.unconscious 
        || player.energy < config.till_energy_cost) {
        return false;
    }

    const auto& slots = player.inventory.get_stacks();
    const auto& selected = slots[player.selected_slot];
    //chequeo que tenga seleccionada la pala
    if (!selected || selected->get_item().get_type() != ItemType::SHOVEL) {
        return false;
    }


    if (x < 0 || y < 0 || x >= farm->width() || y >= farm->height()) {
        return false;
    }

    const int player_x = static_cast<int>(std::floor(player.x));
    const int player_y = static_cast<int>(std::floor(player.y));

    if (std::abs(player_x - x) > 1 || std::abs(player_y - y) > 1) {
        return false;
    }

    const size_t index = static_cast<size_t>(y) * farm->width() + x;
    //verificamos que el terreno sea grass y que no este ni dentro de la cabaña
    //ni dentro de un portal
    if (farm->grid()[index] != Tile::GRASS 
        || farm->cabin().contains_cell(x, y) 
        || farm->portal_at(x + 0.5f, y + 0.5f).has_value()) {
        return false;
    }

    CellPos pos{x, y};

    if (has_placed_object_unlocked(pos)) {
    return false;
    }

    bool inserted = farm_plots.emplace(pos, FarmPlot{}).second;

    if (!inserted) {
        return false;
    }

    player.energy = std::max(0, player.energy - config.till_energy_cost);

    if (player.energy == 0) {
        player.unconscious = true;
        check_day_end_unlocked();
    }

    return true;
}

std::optional<ItemType> Game::crop_type_for_seed(ItemType seed_type) {
    if (seed_type == ItemType::STRAWBERRY_SEED) {
        return ItemType::STRAWBERRY;
    }

    if (seed_type == ItemType::ONION_SEED) {
        return ItemType::ONION;
    }

    if (seed_type == ItemType::POTATO_SEED) {
        return ItemType::POTATO;
    }

    if (seed_type == ItemType::PUMPKIN_SEED) {
        return ItemType::PUMPKIN;
    }

    return std::nullopt;
}

Quality Game::quality_from_score(int score, const GameConfig& config) {
    if (score >= config.quality_perfect_threshold) {
        return Quality::PERFECT;
    }
    if (score >= config.quality_incredible_threshold) {
        return Quality::INCREDIBLE;
    }
    if (score >= config.quality_excellent_threshold) {
        return Quality::EXCELLENT;
    }
    if (score >= config.quality_good_threshold) {
        return Quality::GOOD;
    }
    return Quality::BASE;
}

int Game::generate_luck(int minimum, int maximum) {
    static thread_local std::mt19937 generator(std::random_device{}());
    std::uniform_int_distribution<int> distribution(minimum, maximum);
    return distribution(generator);
}

bool Game::plant_seed(int client_id, int x, int y) {
    std::lock_guard<std::mutex> lock(mtx);

    if (clock.is_finished()) {
        return false;
    }

    const auto client_it = connected_clients.find(client_id);

    if (client_it == connected_clients.end()) {
        return false;
    }

    Player& player = players.at(client_it->second);

    if (player.zone != ZoneId::FARM_ZONE ||
        player.unconscious ||
        player.energy < config.plant_energy_cost) {
        return false;
    }

    if (x < 0 || y < 0 || x >= farm->width() || y >= farm->height()) {
        return false;
    }

    const int player_x = static_cast<int>(player.x);
    const int player_y = static_cast<int>(player.y);

    if (std::abs(player_x - x) > 1 ||
        std::abs(player_y - y) > 1) {
        return false;
    }

    auto plot_it = farm_plots.find(CellPos{x, y});
    if (plot_it == farm_plots.end() ||
        plot_it->second.has_crop()) {
        return false;
    }

    const auto& slots = player.inventory.get_stacks();
    const auto& selected = slots[player.selected_slot];
    if (!selected) {
        return false;
    }

    const auto crop_type = crop_type_for_seed(selected->get_item().get_type());
    if (!crop_type) {
        return false;
    }

    const int waterings = config.crop_waterings.at(*crop_type);
    const int growth_days = config.crop_growth_days.at(*crop_type);
    const int regrowth_days = config.crop_regrowth_days.at(*crop_type);
    const int tolerated_missed = config.crop_missed_water_tolerance.at(*crop_type);
    const auto removed = player.inventory.remove_item(player.selected_slot, RemoveMode::ONE);

    if (!removed) {
        return false;
    }

    if (!plot_it->second.plant(*crop_type, waterings,
            growth_days, regrowth_days, tolerated_missed)) {
        player.inventory.add_item(removed->get_item(), removed->get_quantity());
        return false;
    }

    player.energy -= config.plant_energy_cost;

    if (player.energy == 0) {
        player.unconscious = true;
        check_day_end_unlocked();
    }

    return true;
}

bool Game::water_crop(int client_id, int x, int y) {
    std::lock_guard<std::mutex> lock(mtx);

    if (clock.is_finished()) {
        return false;
    }

    const auto client_it = connected_clients.find(client_id);

    if (client_it == connected_clients.end()) {
        return false;
    }

    Player& player = players.at(client_it->second);

    if (player.zone != ZoneId::FARM_ZONE ||
        player.unconscious ||
        player.energy < config.water_energy_cost) {
        return false;
    }

    if (x < 0 || y < 0 ||
        x >= farm->width() || y >= farm->height()) {
        return false;
    }

    const int player_x = static_cast<int>(player.x);
    const int player_y = static_cast<int>(player.y);

    if (std::abs(player_x - x) > 1 ||
        std::abs(player_y - y) > 1) {
        return false;
    }

    const auto& slots = player.inventory.get_stacks();

    if (player.selected_slot < 0 ||
        static_cast<size_t>(player.selected_slot) >= slots.size()) {
        return false;
    }

    const auto& selected =
        slots[static_cast<size_t>(player.selected_slot)];

    if (!selected ||
        selected->get_item().get_type() != ItemType::WATERING_CAN) {
        return false;
    }

    auto plot_it = farm_plots.find(CellPos{x, y});

    if (plot_it == farm_plots.end()) {
        return false;
    }

    if (!plot_it->second.water(clock.elapsed_game_minutes())) {
        return false;
    }

    player.energy -= config.water_energy_cost;

    if (player.energy == 0) {
        player.unconscious = true;
        check_day_end_unlocked();
    }

    return true;
}



bool Game::harvest_crop(int client_id, int x, int y) {
    std::lock_guard<std::mutex> lock(mtx);

    if (clock.is_finished()) {
        return false;
    }

    const auto client_it = connected_clients.find(client_id);

    if (client_it == connected_clients.end()) {
        return false;
    }

    Player& player = players.at(client_it->second);

    if (player.zone != ZoneId::FARM_ZONE ||
        player.unconscious ||
        player.energy < config.harvest_energy_cost) {
        return false;
    }

    if (x < 0 || y < 0 ||
        x >= farm->width() || y >= farm->height()) {
        return false;
    }

    const int player_x = static_cast<int>(player.x);
    const int player_y = static_cast<int>(player.y);

    if (std::abs(player_x - x) > 1 ||
        std::abs(player_y - y) > 1) {
        return false;
    }

    auto plot_it = farm_plots.find(CellPos{x, y});

    if (plot_it == farm_plots.end()) {
        return false;
    }

    const Crop* crop = plot_it->second.get_crop();

    if (!crop || !crop->is_ready_to_harvest()) {
        return false;
    }

    const int luck_percent = generate_luck(
        config.quality_luck_min_pct,
        config.quality_luck_max_pct
    );

    const int score = crop->quality_score(
        clock.elapsed_game_minutes(),
        luck_percent,
        config.quality_watering_minutes_per_point,
        config.quality_missed_watering_penalty,
        config.quality_harvest_minutes_per_point
    );

    const Quality quality = quality_from_score(score, config);

    const Item item(crop->get_type(), quality);

    if (!player.inventory.can_add_item(item, 1)) {
        return false;
    }

    if (!plot_it->second.harvest()) {
        return false;
    }

    if (player.inventory.add_item(item, 1) != 0) {
        throw std::logic_error(
            "No se pudo almacenar la cosecha");
    }

    player.energy -= config.harvest_energy_cost;

    if (player.energy == 0) {
        player.unconscious = true;
        check_day_end_unlocked();
    }

    return true;
}

std::vector<FarmPlotState> Game::farm_plots_state() const {
    std::vector<FarmPlotState> states;

    for (const auto& [pos, plot] : farm_plots) {
        FarmPlotState state{};

        state.cell_x = static_cast<uint16_t>(pos.x);
        state.cell_y = static_cast<uint16_t>(pos.y);

        const Crop* crop = plot.get_crop();

        if (crop) {
            state.has_crop = true;
            state.crop_type = crop->get_type();

            state.days_grown =
                static_cast<uint16_t>(crop->get_days_grown());

            state.growth_days =
                static_cast<uint16_t>(crop->get_growth_days());

            state.needs_water =
                crop->needs_water(clock.elapsed_game_minutes());

            state.ready_to_harvest =
                crop->is_ready_to_harvest();
        }

        states.push_back(state);
    }

    return states;
}

bool Game::is_decoration(ItemType type) {
    switch (type) {
        case ItemType::CHAIR:
        case ItemType::BARREL_CLOSED:
        case ItemType::BARREL_SWORDS:
        case ItemType::BARREL_WATER:
        case ItemType::CRATE_1:
        case ItemType::CRATE_2:
        case ItemType::FLOWERS_1:
        case ItemType::FLOWERS_2:
        case ItemType::PICTURE_1:
        case ItemType::PICTURE_2:
        case ItemType::WELL:
        case ItemType::STATUE:
        case ItemType::BEAM:
            return true;

        default:
            return false;
    }
}

bool Game::has_placed_object_unlocked(const CellPos& pos) const {
    return decorations.find(pos) != decorations.end() 
           || chests.find(pos) != chests.end();
}

bool Game::can_place_object_unlocked(int x, int y) const {
    if (x < 0 || y < 0 || x >= farm->width() || y >= farm->height()) {
        return false;
    }

    const size_t index = static_cast<size_t>(y) * farm->width() + x;

    const Tile tile = farm->grid()[index];

    if (tile != Tile::GRASS && tile != Tile::FLOOR) {
        return false;
    }

    const CellPos pos{x, y};

    if (has_placed_object_unlocked(pos) ||
        farm_plots.find(pos) != farm_plots.end() ||
        farm->portal_at(x + 0.5f, y + 0.5f)) {
        return false;
    }

    // No colocar sobre camas
    for (const auto& [bed_x, bed_y] : farm->beds()) {
        if (std::floor(bed_x) == x &&
            std::floor(bed_y) == y) {
            return false;
        }
    }

    // No colocar sobre jugadores
    for (const auto& entry : players) {
        const Player& player = entry.second;

        if (player.zone == ZoneId::FARM_ZONE &&
            std::floor(player.x) == x &&
            std::floor(player.y) == y) {
            return false;
        }
    }

    // No bloquear posiciones de llegada de portales
    for (const auto& entry : world->zones_data()) {
        for (const Portal& portal : entry.second->get_portals()) {
            if (portal.destination == ZoneId::FARM_ZONE &&
                std::floor(portal.arrival.first) == x &&
                std::floor(portal.arrival.second) == y) {
                return false;
            }
        }
    }

    return true;
}

bool Game::place_object(int client_id, int x, int y) {
    std::lock_guard<std::mutex> lock(mtx);

    if (clock.is_finished()) {
        return false;
    }

    auto client_it = connected_clients.find(client_id);

    if (client_it == connected_clients.end()) {
        return false;
    }

    Player& player = players.at(client_it->second);

    if (player.zone != ZoneId::FARM_ZONE ||
        player.unconscious) {
        return false;
    }

    if (!can_place_object_unlocked(x, y)) {
        return false;
    }

    const int player_x =
        static_cast<int>(std::floor(player.x));

    const int player_y =
        static_cast<int>(std::floor(player.y));

    if (std::abs(player_x - x) > 1 ||
        std::abs(player_y - y) > 1) {
        return false;
    }

    const auto& slots = player.inventory.get_stacks();
    const int slot = player.selected_slot;

    if (slot < 0 ||
        static_cast<size_t>(slot) >= slots.size() ||
        !slots[slot]) {
        return false;
    }

    const ItemType type =
        slots[slot]->get_item().get_type();

    const CellPos pos{x, y};

    if (type == ItemType::CHEST) {
        chests.try_emplace(pos);

    } else if (is_decoration(type)) {
        decorations.emplace(pos, type);

    } else {
        return false;
    }

    if (!player.inventory.remove_item(slot, RemoveMode::ONE)) {
        if (type == ItemType::CHEST) {
            chests.erase(pos);
        } else {
            decorations.erase(pos);
        }

        return false;
    }

    return true;
}

std::vector<PlacedObjectState> Game::placed_objects_state() const {
    std::vector<PlacedObjectState> result;

    result.reserve(decorations.size() + chests.size());

    for (const auto& [pos, type] : decorations) {
        result.push_back({
            static_cast<uint16_t>(pos.x),
            static_cast<uint16_t>(pos.y),
            type
        });
    }

    for (const auto& entry : chests) {
        const CellPos& pos = entry.first;

        result.push_back({
            static_cast<uint16_t>(pos.x),
            static_cast<uint16_t>(pos.y),
            ItemType::CHEST
        });
    }

    return result;
}

bool Game::near_chest_unlocked(const Player& player, const CellPos& pos) const {

    if (player.zone != ZoneId::FARM_ZONE) {
        return false;
    }

    const int player_x = static_cast<int>(player.x);
    const int player_y = static_cast<int>(player.y);

    return std::abs(player_x - pos.x) <= 1 && std::abs(player_y - pos.y) <= 1;
}

bool Game::close_chest_unlocked(int client_id) {
    for (auto& [pos, chest] : chests) {
        if (chest.get_opened_by() == client_id) {
            return chest.close(client_id);
        }
    }

    return false;
}

bool Game::close_chest(int client_id) {
    std::lock_guard<std::mutex> lock(mtx);

    return close_chest_unlocked(client_id);
}

bool Game::open_chest(int client_id, int x, int y) {
    std::lock_guard<std::mutex> lock(mtx);

    if (clock.is_finished()) {
        return false;
    }

    auto client_it = connected_clients.find(client_id);

    if (client_it == connected_clients.end()) {
        return false;
    }

    Player& player = players.at(client_it->second);

    const CellPos pos{x, y};

    if (player.unconscious || !near_chest_unlocked(player, pos)) {
        return false;
    }

    auto chest_it = chests.find(pos);

    if (chest_it == chests.end()) {
        return false;
    }

    Chest& chest = chest_it->second;

    if (chest.is_opened()) {
        return false;
    }

    //antes de abrir un cofre si tenemos otro abierto lo cerramos
    close_chest_unlocked(client_id);

    return chest.open(client_id);
}

void Game::close_unavailable_chests_unlocked() {
    for (auto& [pos, chest] : chests) {
        if (!chest.is_opened()) {
            continue;
        }

        const int client_id = chest.get_opened_by();

        auto client_it = connected_clients.find(client_id);

        if (clock.is_finished() ||
            client_it == connected_clients.end()) {

            chest.close(client_id);
            continue;
        }

        const Player& player = players.at(client_it->second);

        if (player.unconscious ||
            !near_chest_unlocked(player, pos)) {

            chest.close(client_id);
        }
    }
}

Chest* Game::get_open_chest_unlocked(int client_id, const Player& player) {

    if (clock.is_finished() || player.unconscious) {
        return nullptr;
    }

    for (auto& [pos, chest] : chests) {
        if (chest.get_opened_by() == client_id &&
            near_chest_unlocked(player, pos)) {
            return &chest;
        }
    }

    return nullptr;
}

bool Game::deposit_item(int client_id, int slot_index, RemoveMode mode) {
    std::lock_guard<std::mutex> lock(mtx);

    auto client_it = connected_clients.find(client_id);

    if (client_it == connected_clients.end()) {
        return false;
    }

    Player& player = players.at(client_it->second);

    Chest* chest = get_open_chest_unlocked(client_id, player);

    if (!chest) {
        return false;
    }

    return player.inventory.transfer_item(
        chest->storage, slot_index, mode);
}

bool Game::withdraw_item(int client_id, int slot_index, RemoveMode mode) {
    std::lock_guard<std::mutex> lock(mtx);

    auto client_it = connected_clients.find(client_id);

    if (client_it == connected_clients.end()) {
        return false;
    }

    Player& player = players.at(client_it->second);

    Chest* chest = get_open_chest_unlocked(client_id, player);

    if (!chest) {
        return false;
    }

    return chest->storage.transfer_item(player.inventory, slot_index, mode);
}
#ifndef GAME_H
#define GAME_H

#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <vector>
#include <optional>

#include "common_src/farm_zone.h"
#include "common_src/cave_level.h"
#include "common_src/messages.h"
#include "common_src/queue.h"
#include "common_src/world.h"
#include "client_action.h"
#include "enemy.h"
#include "storage.h"
#include "game_clock.h"
#include "save_data_structs.h"
#include "common_src/character_appearance.h"
#include "game_config.h"
#include "item_catalog.h"
#include "merchant.h"
#include "common_src/cell_pos.h"
#include "farm_plot.h"
#include "enemy_config.h"
#include "random.h"
#include "drop_table.h"
#include "fight_set_tracker.h"
#include "enemy_spawner.h"
#include "projectile.h"
#include "chest.h"

//const int MAX_HEALTH = 100; //elim
const int WEAKENED_HEALTH_PERCENT = 20;
//const int SWORD_DAMAGE = 25; //elim
const float PLAYER_ATTACK_COOLDOWN = 0.5f;  // segundos
//const int MAX_ENERGY = 100; //elim
const int ATTACK_ENERGY_COST = 2;

inline bool is_weakened(int health, int max_health) {
    return health > 0 && health * 100 <= max_health * WEAKENED_HEALTH_PERCENT;
}

struct Player {
    float x;
    float y;
    int health;
    int energy;
    bool unconscious;
    Direction facing;
    float attack_cooldown_left;
    Storage inventory{9};
    int selected_slot = 0;
    int coins;
    ZoneId zone;
    CharacterAppearance appearance;
    std::optional<MerchantType> active_merchant;

Player(float x, float y, int health, int initial_energy,
       int initial_coins, ZoneId zone,
       CharacterAppearance appearance_param) :
    x(x), y(y), health(health), energy(initial_energy),
    unconscious(health == 0 || initial_energy == 0),
    facing(Direction::DOWN), attack_cooldown_left(0.0f),
    inventory(9), coins(initial_coins), zone(zone),
    appearance(appearance_param) {}
};

class Game {
private:
    class EnemyView;
    enum class MovementResult {
        BLOCKED,
        MOVED,
        TRANSPORTED
    };

    std::shared_ptr<const World> world;
    std::shared_ptr<const FarmZone> farm;
    GameConfig config;
    EnemyConfig enemy_config;
    ItemCatalog item_catalog;
    std::map<MerchantType, std::unique_ptr<Merchant>> merchants;
    GameClock clock;
    std::vector<std::unique_ptr<Enemy>> enemies;
    Random rng;
    DropTable drop_table;
    FightSetTracker fight_sets;
    EnemySpawner spawner;
    std::vector<std::unique_ptr<Projectile>> projectiles;
    std::map<std::string, Player> players;          // nombre -> jugador
    std::map<int, std::string> connected_clients;   // client_id -> nombre
    std::set<int> ready_clients;
    bool multiplayer_enabled = false;
    mutable std::mutex mtx;
    Queue<std::shared_ptr<ClientAction>> actions;
    std::map<CellPos, FarmPlot> farm_plots;
    std::map<CellPos, ItemType> decorations;
    std::map<CellPos, Chest> chests;

    std::optional<ItemType> crop_type_for_seed(ItemType seed_type);
    int generate_luck(int minimum, int maximum);
    Quality quality_from_score(int score, const GameConfig& config);
    std::vector<FarmPlotState> farm_plots_state() const;

    // Versiones SIN lock: solo se llaman con mtx ya tomado
    
    bool damage_player_unlocked(const std::string& player_name, int amount);
    bool nearest_player_unlocked(ZoneId zone, float x, float y, EnemyWorld::Target& target) const;
    bool remove_dead_enemies_unlocked();
    uint32_t get_initial_coins() const;
    bool add_player_unlocked(int client_id, const std::string& player_name,
                             ZoneId zone, float x, float y, const CharacterAppearance& appearance);
    std::map<std::string, int> bed_by_player;
    MovementResult move_player_unlocked(const std::string& name, Player& player,
                                        float new_x, float new_y);

    std::vector<PlayerState> snapshot(ZoneId zone, const std::string& self_name) const;
    std::vector<MapPlayerState> map_snapshot() const;
    
    std::vector<EnemyState> enemy_snapshot(ZoneId zone) const;
    std::vector<ProjectileState> projectile_snapshot(ZoneId zone) const;
    bool player_incident_near_unlocked(ZoneId zone, float x, float y, float radius) const;
    void handle_enemy_killed_unlocked(Enemy& enemy, const std::string& killer);
    InventoryState inventory_state(int client_id) const;
    TimeState time_state() const;

    void check_day_end_unlocked();

    //interacciones con npcs
    bool near_merchant_unlocked(const Player& player, MerchantType merchant) const;
    void update_robin_catalog_unlocked();

    //interacciones con objetos
    bool is_in_own_bed_unlocked(const std::string& player_name, const Player& player) const;
    bool has_placed_object_unlocked(const CellPos& pos) const;
    bool can_place_object_unlocked(int x, int y) const;
    bool is_decoration(ItemType type);
    std::vector<PlacedObjectState> placed_objects_state() const;
    //cofres
    bool near_chest_unlocked(const Player& player, const CellPos& pos) const;
    bool close_chest_unlocked(int client_id);
    void close_unavailable_chests_unlocked();
    Chest* get_open_chest_unlocked(int client_id, const Player& player);

public:
    explicit Game(std::shared_ptr<const World> world, const GameConfig& config,
                  const EnemyConfig& enemy_config);

    bool add_player_auto(int client_id, const std::string& player_name, const CharacterAppearance& appearance);
    bool enable_multiplayer();
    bool is_multiplayer() const;
    bool move_player(int client_id, Direction direction);
    bool remove_player(int client_id);
    bool is_empty() const;
    std::vector<int> player_ids() const;

    bool damage_player(const std::string& player_name, int amount);
    bool attack(int client_id);
    bool update(float dt);

    void add_enemy(std::unique_ptr<Enemy> enemy);

    bool all_players_ready_no_mutex() const;

    Queue<std::shared_ptr<ClientAction>>& get_actions_queue();
    std::shared_ptr<const World> get_world() const;

    ServerMessage game_state(int client_id) const;

    bool confirm_next_day(int client_id);
    bool all_players_ready() const;
    bool start_next_day_game();
    std::optional<CharacterAppearance> appearance_for(const std::string& player_name) const;
    bool player_name(int client_id, std::string& player_name) const;
    bool name_to_client_id(int& client_id, const std::string& player_name) const;
    GameSaveData get_save_data(const std::string& game_code, const std::string& game_name, 
        const std::string& owner_name, const std::string& world_name, bool multiplayer) const;
    void restore_save_data(const GameSaveData& data);
    bool select_slot(int client_id, int slot_index);
    int get_selected_slot(int client_id) const;

    //compras y ventas
    bool buy_item(int client_id, MerchantType merchant, const std::string& product, int quantity);
    bool sell_item(int client_id, MerchantType merchant, const std::string& product, int quantity);

    //npcs
    bool interact_with_merchant(int client_id, MerchantType merchant);
    std::optional<MerchantType> get_active_merchant(int client_id) const;
    std::optional<std::string> get_merchant_catalog(int client_id) const;
    
    //plantaciones
    bool till_soil(int client_id, int x, int y);
    bool plant_seed(int client_id, int x, int y);
    bool water_crop(int client_id, int x, int y);
    bool harvest_crop(int client_id, int x, int y);

    // Colocacion de objetos
    bool place_object(int client_id, int x, int y);

    // Interacciones con cofres
    bool open_chest(int client_id, int x, int y);
    bool close_chest(int client_id);
    bool deposit_item(int client_id, int slot_index, RemoveMode mode);
    bool withdraw_item(int client_id, int slot_index, RemoveMode mode);
    
};

#endif

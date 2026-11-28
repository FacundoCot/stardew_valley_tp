#ifndef ZONE_H
#define ZONE_H

#include <optional>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

struct Rectangle {
    int x;
    int y;
    int width;
    int height;

    bool contains_cell(int column, int row) const;
    bool contains(float px, float py) const;
};

struct ZoneId {
    enum class Kind {
        FARM = 0,
        VILLAGE = 1,
        CAVE_LEVEL = 2,
        HOUSE = 3,
        COMMUNITY_CENTER = 4
    };

    enum class AreaType {
        MARKET,
        COMMUNITY_CENTER
    };

    struct ZoneArea {
        AreaType type;
        int x;
        int y;
        int width;
        int height;
    };

    Kind kind;
    int level;

    constexpr bool operator<(const ZoneId& other) const {
        return std::pair<Kind, int>(kind, level) <
               std::pair<Kind, int>(other.kind, other.level);
    }

    constexpr bool operator==(const ZoneId& other) const = default;

    static const ZoneId FARM_ZONE;
    static const ZoneId VILLAGE_ZONE;
    static const ZoneId HOUSE_ZONE;
    static const ZoneId COMMUNITY_CENTER_ZONE;
};

inline const ZoneId ZoneId::FARM_ZONE{ZoneId::Kind::FARM, 0};
inline const ZoneId ZoneId::VILLAGE_ZONE{ZoneId::Kind::VILLAGE, 0};
inline const ZoneId ZoneId::HOUSE_ZONE{ZoneId::Kind::HOUSE, 0};
inline const ZoneId ZoneId::COMMUNITY_CENTER_ZONE{
    ZoneId::Kind::COMMUNITY_CENTER, 0};

enum class Tile {
    WALL = 0,
    FLOOR = 1,
    GRASS = 2,
    PATH = 3,
    TREE = 4
};

inline bool is_walkable_tile(Tile tile) {
    return tile == Tile::FLOOR || tile == Tile::GRASS || tile == Tile::PATH;
}

enum class PortalType {
    STAIR = 0,
    DUNGEON_DOOR = 1,
    DUNGEON_EXIT = 2,
    ROAD = 3,
    HOUSE_DOOR = 4,
    HOUSE_EXIT = 5,
    CENTER_DOOR = 6,
    CENTER_EXIT = 7
};

enum class StaticElementType : uint8_t {
    BED = 0,
    PIERRE = 1,
    MARNIE = 2,
    CLINT = 3,
    ROBIN = 4,
    ENEMY_SPAWN = 5,
    ORE_COAL = 6,
    ORE_LAPIS = 7,
    ORE_SILVER = 8,
    ORE_GOLD = 9,
    TRAP_SPIKES = 10,
    TRAP_FLAME_LEFT = 11,
    TRAP_FLAME_RIGHT = 12,
    TRAP_FLAME_DOWN = 13,
    ABANDONED_CHEST = 14,
    OFFERING_CROPS = 15,
    OFFERING_METALS = 16,
    OFFERING_ANIMALS = 17,
    OFFERING_FIGHT = 18,
    OFFERING_MONEY = 19,
    TRAP_FLAME_UP = 20
};

struct StaticElement {
    StaticElementType type;
    std::pair<int, int> cell;
};

struct VisualTile {
    int x;
    int y;
    std::string layer;
    std::string sheet;
    int col;
    int row;
};

struct Portal {
    PortalType type;
    std::pair<int, int> cell;
    ZoneId destination;
    std::pair<float, float> arrival;
};

class TileGrid {
private:
    int grid_width;
    int grid_height;
    std::vector<Tile> tiles;

public:
    TileGrid(int width, int height, std::vector<Tile> tiles);

    int width() const;
    int height() const;
    bool is_walkable(float x, float y) const;
    std::pair<int, int> cell(float x, float y) const;
    const std::vector<Tile>& tiles_data() const;
};

class Zone {
private:
    std::vector<Portal> portals;
    std::vector<StaticElement> static_elements;
    std::vector<VisualTile> visual_tiles;

protected:
    void validate_portals(const TileGrid& grid) const;

public:
    explicit Zone(std::vector<Portal> portals);
    virtual ~Zone() = default;

    virtual ZoneId id() const = 0;
    virtual int width() const = 0;
    virtual int height() const = 0;
    virtual bool is_walkable(float x, float y) const = 0;

    const std::vector<Portal>& get_portals() const;
    const std::vector<StaticElement>& get_static_elements() const;
    const std::vector<VisualTile>& get_visual_tiles() const;
    void set_static_elements(std::vector<StaticElement> elements);
    void set_visual_tiles(std::vector<VisualTile> tiles);
    std::optional<Portal> portal_at(float x, float y) const;
};

#endif

#ifndef ZONE_H
#define ZONE_H

#include <optional>
#include <utility>
#include <vector>

struct Rect {
    int x;
    int y;
    int width;
    int height;

    bool contains_cell(int column, int row) const;
    bool contains(float px, float py) const;
};

struct ZoneId {
    enum class Kind {
        FARM,
        VILLAGE,
        CAVE_LEVEL
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
};

inline const ZoneId ZoneId::FARM_ZONE{ZoneId::Kind::FARM, 0};
inline const ZoneId ZoneId::VILLAGE_ZONE{ZoneId::Kind::VILLAGE, 0};

enum class Tile {
    WALL,
    FLOOR
};

enum class PortalType {
    STAIR,
    DUNGEON_DOOR,
    DUNGEON_EXIT,
    ROAD
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
    bool is_floor(float x, float y) const;
    std::pair<int, int> cell(float x, float y) const;
    const std::vector<Tile>& tiles_data() const;
};

class Zone {
private:
    std::vector<Portal> portals;

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
    std::optional<Portal> portal_at(float x, float y) const;
};

#endif

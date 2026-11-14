#ifndef CAVE_LEVEL_H
#define CAVE_LEVEL_H

#include <utility>
#include <vector>

#include "zone.h"

class CaveLevel : public Zone {
private:
    int level;
    TileGrid tile_grid;
    std::vector<std::pair<float, float>> spawn_points;

public:
    CaveLevel(int level, int width, int height, std::vector<Tile> tiles,
              std::vector<Portal> portals,
              std::vector<std::pair<float, float>> enemy_spawns);

    ZoneId id() const override;
    int width() const override;
    int height() const override;
    bool is_walkable(float x, float y) const override;
    const std::vector<std::pair<float, float>>& enemy_spawns() const;
};

#endif

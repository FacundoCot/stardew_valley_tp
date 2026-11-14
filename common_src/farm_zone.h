#ifndef FARM_ZONE_H
#define FARM_ZONE_H

#include <utility>
#include <vector>

#include "zone.h"

class FarmZone : public Zone {
private:
    TileGrid tile_grid;
    Rect cabin_position;
    std::vector<std::pair<float, float>> bed_spots;

public:
    FarmZone(int width, int height, std::vector<Tile> tiles,
             Rect cabin,
             std::vector<std::pair<float, float>> bed_spots,
             std::vector<Portal> portals = {});

    ZoneId id() const override;
    int width() const override;
    int height() const override;
    bool is_walkable(float x, float y) const override;

    const std::vector<Tile>& grid() const;
    Rect cabin() const;
    int bed_count() const;
    std::pair<float, float> bed_position(int bed_index) const;
};

#endif

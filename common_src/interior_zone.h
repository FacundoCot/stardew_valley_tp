#ifndef INTERIOR_ZONE_H
#define INTERIOR_ZONE_H

#include "zone.h"

class InteriorZone : public Zone {
private:
    ZoneId zone_id;
    TileGrid tile_grid;
    std::vector<std::pair<float, float>> beds_cells;

public:
    InteriorZone(ZoneId id, int width, int height, std::vector<Tile> tiles,
                 std::vector<Portal> portals,
                 std::vector<std::pair<float, float>> beds = {});

    ZoneId id() const override;
    int width() const override;
    int height() const override;
    bool is_walkable(float x, float y) const override;

    const std::vector<Tile>& grid() const;
    const std::vector<std::pair<float, float>>& beds() const;
    std::pair<float, float> bed_position(int bed_index) const;
};

#endif

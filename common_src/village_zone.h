#ifndef VILLAGE_ZONE_H
#define VILLAGE_ZONE_H

#include <vector>

#include "zone.h"

class VillageZone : public Zone {
private:
    TileGrid tile_grid;
    std::vector<ZoneId::ZoneArea> zone_areas;

public:
    VillageZone(int width, int height, std::vector<Tile> tiles,
                std::vector<Portal> portals,
                std::vector<ZoneId::ZoneArea> areas);

    ZoneId id() const override;
    int width() const override;
    int height() const override;
    bool is_walkable(float x, float y) const override;

    const std::vector<ZoneId::ZoneArea>& areas() const;
};

#endif

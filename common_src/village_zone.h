#ifndef VILLAGE_ZONE_H
#define VILLAGE_ZONE_H

#include <vector>

#include "zone.h"
#include "merchant_pos.h"

class VillageZone : public Zone {
private:
    TileGrid tile_grid;
    std::vector<ZoneId::ZoneArea> zone_areas;
    std::vector<MerchantPosition> merchant_positions;

    void validate_merchants() const;

public:
    VillageZone(int width, int height, std::vector<Tile> tiles,
                std::vector<Portal> portals,
                std::vector<ZoneId::ZoneArea> areas,
                std::vector<MerchantPosition> merchants);

    ZoneId id() const override;
    int width() const override;
    int height() const override;
    bool is_walkable(float x, float y) const override;

    const std::vector<Tile>& grid() const;
    const std::vector<ZoneId::ZoneArea>& areas() const;
    const std::vector<MerchantPosition>& merchants() const;
};

#endif

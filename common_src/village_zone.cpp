#include "village_zone.h"

#include <cstddef>
#include <set>
#include <stdexcept>
#include <utility>

VillageZone::VillageZone(int width, int height, std::vector<Tile> tiles,
                         std::vector<Portal> portals,
                         std::vector<ZoneId::ZoneArea> areas_param):
        Zone(std::move(portals)),
        tile_grid(width, height, std::move(tiles)),
        zone_areas(std::move(areas_param)) {
    validate_portals(tile_grid);

    for (const Portal& portal : get_portals()) {
        if (portal.type != PortalType::ROAD &&
            portal.type != PortalType::DUNGEON_DOOR) {
            throw std::invalid_argument(
                "el pueblo solo puede tener caminos y puerta de mazmorra");
        }
    }

    for (const ZoneId::ZoneArea& area : zone_areas) {
        if (area.width < 1 || area.height < 1 || area.x < 0 || area.y < 0 ||
            area.x + area.width > width || area.y + area.height > height) {
            throw std::invalid_argument("el area del pueblo esta fuera de la grilla");
        }
        for (int row = area.y; row < area.y + area.height; ++row) {
            for (int column = area.x; column < area.x + area.width; ++column) {
                if (!tile_grid.is_floor(column + 0.5f, row + 0.5f)) {
                    throw std::invalid_argument(
                        "todas las celdas del area deben ser piso");
                }
                for (const Portal& portal : get_portals()) {
                    if (portal.cell == std::pair<int, int>{column, row}) {
                        throw std::invalid_argument(
                            "un portal no puede estar dentro de un area");
                    }
                }
            }
        }
    }

    for (size_t first = 0; first < zone_areas.size(); ++first) {
        const ZoneId::ZoneArea& left = zone_areas[first];
        for (size_t second = first + 1; second < zone_areas.size(); ++second) {
            const ZoneId::ZoneArea& right = zone_areas[second];
            const bool separated =
                left.x + left.width <= right.x ||
                right.x + right.width <= left.x ||
                left.y + left.height <= right.y ||
                right.y + right.height <= left.y;
            if (!separated) {
                throw std::invalid_argument("dos areas del pueblo se superponen");
            }
        }
    }
}

ZoneId VillageZone::id() const {
    return ZoneId::VILLAGE_ZONE;
}

int VillageZone::width() const {
    return tile_grid.width();
}

int VillageZone::height() const {
    return tile_grid.height();
}

bool VillageZone::is_walkable(float x, float y) const {
    return tile_grid.is_floor(x, y);
}

const std::vector<ZoneId::ZoneArea>& VillageZone::areas() const {
    return zone_areas;
}

#include "village_zone.h"

#include <cstddef>
#include <set>
#include <stdexcept>
#include <utility>

VillageZone::VillageZone(int width, int height, std::vector<Tile> tiles,
                         std::vector<Portal> portals,
                         std::vector<ZoneId::ZoneArea> areas_param,
                         std::vector<MerchantPosition> merchants_param,
                         std::optional<Rectangle> community_center_param):
        Zone(std::move(portals)),
        tile_grid(width, height, std::move(tiles)),
        zone_areas(std::move(areas_param)),
        merchant_positions(std::move(merchants_param)),
        community_center_position(community_center_param) {
    validate_portals(tile_grid);
    validate_merchants();

    for (const Portal& portal : get_portals()) {
        if (portal.type != PortalType::ROAD &&
            portal.type != PortalType::DUNGEON_DOOR &&
            portal.type != PortalType::CENTER_DOOR) {
            throw std::invalid_argument(
                "tipo de portal invalido para el pueblo");
        }
    }

    if (community_center_position.has_value()) {
        const Rectangle& center = *community_center_position;
        if (center.width < 1 || center.height < 1 || center.x < 0 ||
            center.y < 0 || center.width > width || center.height > height ||
            center.x > width - center.width ||
            center.y > height - center.height) {
            throw std::invalid_argument(
                "el centro comunitario esta fuera de la grilla del pueblo");
        }
    }

    for (const ZoneId::ZoneArea& area : zone_areas) {
        if (area.width < 1 || area.height < 1 || area.x < 0 || area.y < 0 ||
            area.x + area.width > width || area.y + area.height > height) {
            throw std::invalid_argument("el area del pueblo esta fuera de la grilla");
        }
        for (int row = area.y; row < area.y + area.height; ++row) {
            for (int column = area.x; column < area.x + area.width; ++column) {
                if (!tile_grid.is_walkable(column + 0.5f, row + 0.5f)) {
                    throw std::invalid_argument(
                        "todas las celdas del area deben ser caminables");
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
    return tile_grid.is_walkable(x, y);
}

const std::vector<Tile>& VillageZone::grid() const {
    return tile_grid.tiles_data();
}

const std::vector<ZoneId::ZoneArea>& VillageZone::areas() const {
    return zone_areas;
}

const std::vector<MerchantPosition>& VillageZone::merchants() const {
    return merchant_positions;
}

std::optional<Rectangle> VillageZone::community_center() const {
    return community_center_position;
}

void VillageZone::validate_merchants() const {
    std::set<MerchantType> seen_merchants;

    for (const MerchantPosition& merchant : merchant_positions) {
        // validamos que este en una posicion caminable
        if (!tile_grid.is_walkable(merchant.x, merchant.y)) {
            throw std::invalid_argument("Posicion de comerciante invalida");
        }

        // validamos que no se repitan los comerciantes
        if (!seen_merchants.insert(merchant.type).second) {
            throw std::invalid_argument("Comerciante duplicado");
        }

        const auto cell = tile_grid.cell(merchant.x, merchant.y);

        // vemos que no esten encima de portales
        for (const Portal& portal : get_portals()) {
            if (portal.cell == cell) {
                throw std::invalid_argument("Comerciante sobre un portal");
            }
        }
    }
}
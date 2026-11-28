#include "interior_zone.h"

#include <stdexcept>
#include <utility>

InteriorZone::InteriorZone(ZoneId id_param, int width, int height,
                           std::vector<Tile> tiles,
                           std::vector<Portal> portals,
                           std::vector<std::pair<float, float>> beds_param):
        Zone(std::move(portals)),
        zone_id(id_param),
        tile_grid(width, height, std::move(tiles)),
        beds_cells(std::move(beds_param)) {
    if ((zone_id.kind != ZoneId::Kind::HOUSE &&
         zone_id.kind != ZoneId::Kind::COMMUNITY_CENTER) ||
        zone_id.level != 0) {
        throw std::invalid_argument("id de zona interior invalido");
    }

    validate_portals(tile_grid);
    for (const Portal& portal : get_portals()) {
        const PortalType expected =
            zone_id.kind == ZoneId::Kind::HOUSE ?
                PortalType::HOUSE_EXIT : PortalType::CENTER_EXIT;
        if (portal.type != expected) {
            throw std::invalid_argument(
                "tipo de portal invalido para una zona interior");
        }
    }
    if (zone_id.kind == ZoneId::Kind::HOUSE) {
        if (beds_cells.empty()) {
            throw std::invalid_argument("la casa debe tener una cama");
        }
        if (beds_cells.size() != 1) {
            throw std::invalid_argument("la casa debe tener exactamente una cama");
        }
        const auto [x, y] = beds_cells.front();
        if (!tile_grid.is_walkable(x, y)) {
            throw std::invalid_argument("la cama debe estar sobre un tile caminable");
        }
    } else if (!beds_cells.empty()) {
        throw std::invalid_argument("solo la casa puede tener camas");
    }
}

ZoneId InteriorZone::id() const {
    return zone_id;
}

int InteriorZone::width() const {
    return tile_grid.width();
}

int InteriorZone::height() const {
    return tile_grid.height();
}

bool InteriorZone::is_walkable(float x, float y) const {
    return tile_grid.is_walkable(x, y);
}

const std::vector<Tile>& InteriorZone::grid() const {
    return tile_grid.tiles_data();
}

const std::vector<std::pair<float, float>>& InteriorZone::beds() const {
    return beds_cells;
}

std::pair<float, float> InteriorZone::bed_position(int bed_index) const {
    if (bed_index != 0 || beds_cells.empty()) {
        throw std::out_of_range("indice de cama fuera de rango");
    }
    return beds_cells.front();
}

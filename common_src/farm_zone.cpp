#include "farm_zone.h"

#include <cmath>
#include <cstddef>
#include <set>
#include <stdexcept>
#include <utility>

namespace {
std::pair<int, int> cell_of(float x, float y) {
    return {static_cast<int>(std::floor(x)), static_cast<int>(std::floor(y))};
}

std::pair<float, float> cell_center(float x, float y) {
    return {std::floor(x) + 0.5f, std::floor(y) + 0.5f};
}
}  // namespace

FarmZone::FarmZone(int width, int height, std::vector<Tile> tiles,
                   Rect cabin,
                   std::vector<std::pair<float, float>> bed_spots_param,
                   std::vector<Portal> portals):
        Zone(std::move(portals)),
        tile_grid(width, height, std::move(tiles)),
        cabin_position(cabin),
        bed_spots(std::move(bed_spots_param)) {
    if (cabin.width < 1 || cabin.height < 1 || cabin.x < 0 ||
        cabin.y < 0 || cabin.x + cabin.width > width ||
        cabin.y + cabin.height > height) {
        throw std::invalid_argument("la cabania esta fuera de la grilla");
    }

    for (int row = cabin.y; row < cabin.y + cabin.height; ++row) {
        for (int column = cabin.x; column < cabin.x + cabin.width; ++column) {
            if (!tile_grid.is_floor(column + 0.5f, row + 0.5f)) {
                throw std::invalid_argument("la cabania debe estar en el piso");
            }
        }
    }
    if (bed_spots.empty()) {
        throw std::invalid_argument("la granja debe tener camas");
    }

    std::set<std::pair<int, int>> occupied;
    for (auto& bed : bed_spots) {
        const auto cell = cell_of(bed.first, bed.second);
        if (!cabin.contains(bed.first, bed.second) ||
            !tile_grid.is_floor(bed.first, bed.second)) {
            throw std::invalid_argument(
                "la cama debe estar dentro de la cabania y en piso");
        }
        if (!occupied.insert(cell).second) {
            throw std::invalid_argument("dos camas ocupan la misma celda");
        }
        bed = cell_center(bed.first, bed.second);
    }

    validate_portals(tile_grid);
    for (const Portal& portal : get_portals()) {
        if (portal.type != PortalType::DUNGEON_DOOR &&
            portal.type != PortalType::ROAD) {
            throw std::invalid_argument(
                "la granja solo puede tener puertas de mazmorra y caminos");
        }
        if (cabin.contains_cell(portal.cell.first, portal.cell.second)) {
            throw std::invalid_argument(
                "un portal no puede estar dentro de la cabania");
        }
    }
}

ZoneId FarmZone::id() const {
    return ZoneId::FARM_ZONE;
}

int FarmZone::width() const {
    return tile_grid.width();
}

int FarmZone::height() const {
    return tile_grid.height();
}

bool FarmZone::is_walkable(float x, float y) const {
    return tile_grid.is_floor(x, y);
}

const std::vector<Tile>& FarmZone::grid() const {
    return tile_grid.tiles_data();
}

Rect FarmZone::cabin() const {
    return cabin_position;
}

int FarmZone::bed_count() const {
    return static_cast<int>(bed_spots.size());
}

std::pair<float, float> FarmZone::bed_position(int bed_index) const {
    if (bed_index < 0 || bed_index >= bed_count()) {
        throw std::out_of_range("indice de cama fuera de rango");
    }
    return bed_spots[static_cast<size_t>(bed_index)];
}

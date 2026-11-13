#include "zone.h"

#include <cmath>
#include <cstddef>
#include <map>
#include <stdexcept>
#include <utility>

bool Rect::contains_cell(int column, int row) const {
    return column >= x && row >= y && column < x + width &&
           row < y + height;
}

bool Rect::contains(float px, float py) const {
    return contains_cell(static_cast<int>(std::floor(px)),
                         static_cast<int>(std::floor(py)));
}

TileGrid::TileGrid(int width, int height, std::vector<Tile> tiles_param):
        grid_width(width),
        grid_height(height),
        tiles(std::move(tiles_param)) {
    if (width <= 0 || height <= 0 ||
        tiles.size() != static_cast<size_t>(width) *
                             static_cast<size_t>(height)) {
        throw std::invalid_argument("la grilla no es valida");
    }
}

int TileGrid::width() const {
    return grid_width;
}

int TileGrid::height() const {
    return grid_height;
}

bool TileGrid::is_floor(float x, float y) const {
    if (!std::isfinite(x) || !std::isfinite(y)) {
        return false;
    }
    const auto [column, row] = cell(x, y);
    return column >= 0 && row >= 0 && column < grid_width && row < grid_height &&
           tiles[static_cast<size_t>(row * grid_width + column)] == Tile::FLOOR;
}

std::pair<int, int> TileGrid::cell(float x, float y) const {
    return {static_cast<int>(std::floor(x)), static_cast<int>(std::floor(y))};
}

const std::vector<Tile>& TileGrid::tiles_data() const {
    return tiles;
}

Zone::Zone(std::vector<Portal> portals_param):
        portals(std::move(portals_param)) {}

void Zone::validate_portals(const TileGrid& grid) const {
    std::map<std::pair<int, int>, bool> cells;
    for (const Portal& portal : portals) {
        const auto [column, row] = portal.cell;
        if (column < 0 || row < 0 || column >= grid.width() ||
            row >= grid.height() ||
            grid.tiles_data()[static_cast<size_t>(row * grid.width() + column)] !=
                Tile::FLOOR) {
            throw std::invalid_argument("un portal debe estar sobre piso");
        }
        if (!cells.emplace(portal.cell, true).second) {
            throw std::invalid_argument("hay dos portales en la misma celda");
        }
    }
}

const std::vector<Portal>& Zone::get_portals() const {
    return portals;
}

std::optional<Portal> Zone::portal_at(float x, float y) const {
    const auto cell_position = std::pair<int, int>(
        static_cast<int>(std::floor(x)), static_cast<int>(std::floor(y)));
    for (const Portal& portal : portals) {
        if (portal.cell == cell_position) {
            return portal;
        }
    }
    return std::nullopt;
}

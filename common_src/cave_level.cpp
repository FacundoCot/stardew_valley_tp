#include "cave_level.h"

#include <cmath>
#include <cstddef>
#include <set>
#include <stdexcept>
#include <utility>

CaveLevel::CaveLevel(int level_param, int width, int height,
                     std::vector<Tile> tiles, std::vector<Portal> portals,
                     std::vector<std::pair<float, float>> enemy_spawns_param):
        Zone(std::move(portals)),
        level(level_param),
        tile_grid(width, height, std::move(tiles)),
        spawn_points(std::move(enemy_spawns_param)) {
    if (level < 1 || level > 10) {
        throw std::invalid_argument("el nivel de caverna debe estar entre 1 y 10");
    }
    validate_portals(tile_grid);
    for (const Portal& portal : get_portals()) {
        if (portal.type == PortalType::DUNGEON_DOOR) {
            throw std::invalid_argument(
                "un nivel no puede tener puerta de entrada");
        }
    }

    std::set<std::pair<int, int>> spawn_cells;
    for (const auto& spawn : spawn_points) {
        if (!tile_grid.is_floor(spawn.first, spawn.second)) {
            throw std::invalid_argument("spawns de enemigo invalidos o repetidos");
        }
        const auto cell = tile_grid.cell(spawn.first, spawn.second);
        if (!spawn_cells.insert(cell).second) {
            throw std::invalid_argument("spawns de enemigo invalidos o repetidos");
        }
    }
    if (spawn_points.empty()) {
        throw std::invalid_argument("cada nivel debe tener un spawn de enemigo");
    }
}

ZoneId CaveLevel::id() const {
    return {ZoneId::Kind::CAVE_LEVEL, level};
}

int CaveLevel::width() const {
    return tile_grid.width();
}

int CaveLevel::height() const {
    return tile_grid.height();
}

bool CaveLevel::is_walkable(float x, float y) const {
    return tile_grid.is_floor(x, y);
}

const std::vector<std::pair<float, float>>& CaveLevel::enemy_spawns() const {
    return spawn_points;
}

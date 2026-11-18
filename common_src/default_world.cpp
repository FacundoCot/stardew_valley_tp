#include "default_world.h"

#include <cstddef>
#include <map>
#include <stdexcept>
#include <utility>

#include "cave_level.h"
#include "define.h"
#include "farm_zone.h"
#include "village_zone.h"

namespace {
std::vector<Tile> bordered_grid(int width, int height, Tile interior) {
    std::vector<Tile> tiles(static_cast<size_t>(width) * height, interior);
    for (int row = 0; row < height; ++row) {
        for (int column = 0; column < width; ++column) {
            if (column == 0 || row == 0 || column == width - 1 ||
                row == height - 1) {
                tiles[static_cast<size_t>(row * width + column)] = Tile::WALL;
            }
        }
    }
    return tiles;
}

std::shared_ptr<const Zone> build_farm() {
    auto tiles = bordered_grid(ZONE_WIDTH, ZONE_HEIGHT, Tile::GRASS);
    for (int row = 10; row < 19; ++row) {
        for (int column = 10; column < 22; ++column) {
            tiles[static_cast<size_t>(row * ZONE_WIDTH + column)] = Tile::FLOOR;
        }
    }
    for (int column = 1; column <= 178; ++column) {
        tiles[static_cast<size_t>(60 * ZONE_WIDTH + column)] = Tile::PATH;
    }
    tiles[60 * ZONE_WIDTH + 179] = Tile::PATH;

    // Posiciones fijas de arboles de la granja; dejan libres la cabania,
    // las camas, el camino horizontal y el portal de la mazmorra.
    const std::vector<std::pair<int, int>> farm_trees{
        {30, 20}, {31, 20}, {33, 20},
        {30, 21}, {32, 21}, {33, 21},
        {31, 22}, {32, 22},
        {30, 23}, {33, 23},
        {150, 85}, {151, 85}, {153, 85}, {154, 85},
        {150, 86}, {152, 86}, {154, 86},
        {151, 87}, {153, 87},
        {152, 88}, {154, 88}};
    for (const auto [column, row] : farm_trees) {
        tiles[static_cast<size_t>(row * ZONE_WIDTH + column)] = Tile::TREE;
    }

    std::vector<Portal> portals{
        {PortalType::ROAD, {179, 60}, ZoneId::VILLAGE_ZONE, {1.5f, 60.5f}},
        {PortalType::DUNGEON_DOOR, {90, 70},
         {ZoneId::Kind::CAVE_LEVEL, 1}, {1.5f, 1.5f}}};
    return std::make_shared<const FarmZone>(
        ZONE_WIDTH, ZONE_HEIGHT, std::move(tiles), Rectangle{10, 10, 12, 9},
        std::vector<std::pair<float, float>>{
            {12.5f, 12.5f}, {19.5f, 12.5f},
            {12.5f, 16.5f}, {19.5f, 16.5f}},
        std::move(portals));
}

std::shared_ptr<const Zone> build_village() {
    auto tiles = bordered_grid(ZONE_WIDTH, ZONE_HEIGHT, Tile::GRASS);
    for (int column = 1; column <= 178; ++column) {
        tiles[static_cast<size_t>(60 * ZONE_WIDTH + column)] = Tile::PATH;
    }
    tiles[60 * ZONE_WIDTH] = Tile::PATH;
    // Arboles fijos del pueblo, fuera del camino horizontal central.
    const std::vector<std::pair<int, int>> village_trees{
        {10, 10}, {11, 10}, {12, 10},
        {168, 20}, {169, 20}, {170, 20},
        {20, 100}, {21, 100}, {22, 100}};
    for (const auto [column, row] : village_trees) {
        tiles[static_cast<size_t>(row * ZONE_WIDTH + column)] = Tile::TREE;
    }
    return std::make_shared<const VillageZone>(
        ZONE_WIDTH, ZONE_HEIGHT, std::move(tiles),
        std::vector<Portal>{{PortalType::ROAD, {0, 60},
                             ZoneId::FARM_ZONE, {178.5f, 60.5f}}},
        std::vector<ZoneId::ZoneArea>{});
}

std::shared_ptr<const Zone> build_cave(int level) {
    if (level < 1 || level > 10) {
        throw std::invalid_argument("nivel de caverna fuera de rango");
    }
    auto tiles = bordered_grid(ZONE_WIDTH, ZONE_HEIGHT, Tile::FLOOR);
    for (int row = 55; row <= 64; ++row) {
        for (int column = 85; column <= 94; ++column) {
            tiles[static_cast<size_t>(row * ZONE_WIDTH + column)] = Tile::WALL;
        }
    }
    std::vector<Portal> portals{
        {PortalType::DUNGEON_EXIT, {1, 118}, ZoneId::FARM_ZONE,
         {30.5f, 60.5f}}};
    if (level > 1) {
        portals.push_back({PortalType::STAIR, {1, 1},
                           {ZoneId::Kind::CAVE_LEVEL, level - 1},
                           {177.5f, 117.5f}});
    }
    if (level < 10) {
        portals.push_back({PortalType::STAIR, {178, 118},
                           {ZoneId::Kind::CAVE_LEVEL, level + 1},
                           {2.5f, 2.5f}});
    }
    return std::make_shared<const CaveLevel>(
        level, ZONE_WIDTH, ZONE_HEIGHT, std::move(tiles), std::move(portals),
        std::vector<std::pair<float, float>>{{45.5f, 30.5f}});
}
}  // namespace

std::shared_ptr<const World> build_default_world() {
    std::map<ZoneId, std::shared_ptr<const Zone>> zones;
    zones.emplace(ZoneId::FARM_ZONE, build_farm());
    zones.emplace(ZoneId::VILLAGE_ZONE, build_village());
    for (int level = 1; level <= 10; ++level) {
        const ZoneId id{ZoneId::Kind::CAVE_LEVEL, level};
        zones.emplace(id, build_cave(level));
    }
    return std::make_shared<const World>(std::move(zones));
}

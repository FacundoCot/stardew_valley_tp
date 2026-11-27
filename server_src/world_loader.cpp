#include "world_loader.h"

#include <toml++/toml.hpp>

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <limits>
#include <map>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "../common_src/cave_level.h"
#include "../common_src/farm_zone.h"
#include "../common_src/interior_zone.h"
#include "../common_src/village_zone.h"
#include "../common_src/merchant_pos.h"

namespace {

struct LayerInfo {
    bool solid = false;
};

struct ElementInfo {
    std::string type;
    int x = 0;
    int y = 0;
};

struct ZoneInfo {
    std::string kind;
    int level = 0;
    int width = 0;
    int height = 0;
    std::vector<ElementInfo> elements;
};

std::runtime_error map_error(const std::string& map_name,
                             const std::string& detail) {
    return std::runtime_error("Error cargando mapa '" + map_name + "': " + detail);
}

bool valid_integer(const toml::node& node) {
    return node.is_integer();
}

int read_int(const toml::table& table, const std::string& key,
             const std::string& map_name, const std::string& zone_name,
             bool required = true) {
    const toml::node* node = table.get(key);
    if (node == nullptr) {
        if (required) {
            throw map_error(map_name, "zona '" + zone_name + "' falta el campo '" + key + "'");
        }
        return 0;
    }
    if (!valid_integer(*node)) {
        throw map_error(map_name, "zona '" + zone_name + "' campo '" + key +
                                  "' debe ser entero");
    }
    const auto value = node->value<int>();
    if (!value) {
        throw map_error(map_name, "zona '" + zone_name + "' campo '" + key +
                                  "' fuera de rango");
    }
    return *value;
}

std::string read_string(const toml::table& table, const std::string& key,
                        const std::string& map_name,
                        const std::string& context) {
    const toml::node* node = table.get(key);
    if (node == nullptr || !node->is_string()) {
        throw map_error(map_name, context + " falta o invalido el campo '" + key + "'");
    }
    return node->value_or<std::string>("");
}

Rectangle read_rect(const toml::table& zone, const std::string& key,
                    const std::string& map_name, const std::string& zone_name,
                    int zone_width, int zone_height) {
    const toml::table* rect = zone[key].as_table();
    if (rect == nullptr) {
        throw map_error(map_name, "zona '" + zone_name + "' falta el campo '" + key + "'");
    }
    const int x = read_int(*rect, "x", map_name, zone_name);
    const int y = read_int(*rect, "y", map_name, zone_name);
    const int width = read_int(*rect, "width", map_name, zone_name);
    const int height = read_int(*rect, "height", map_name, zone_name);
    if (width <= 0 || height <= 0 || x < 0 || y < 0 ||
        x > zone_width - width || y > zone_height - height) {
        throw map_error(map_name, "zona '" + zone_name + "' campo '" + key +
                                  "' fuera de los limites");
    }
    return {x, y, width, height};
}

std::pair<float, float> center(const std::pair<int, int>& cell) {
    return {static_cast<float>(cell.first) + 0.5f,
            static_cast<float>(cell.second) + 0.5f};
}

bool contains_element(const std::vector<ElementInfo>& elements,
                      const std::pair<int, int>& cell) {
    return std::any_of(elements.begin(), elements.end(),
                       [&](const ElementInfo& element) {
                           return element.x == cell.first &&
                                  element.y == cell.second;
                       });
}

bool contains_portal(const std::vector<Portal>& portals,
                     const std::pair<int, int>& cell) {
    return std::any_of(portals.begin(), portals.end(),
                       [&](const Portal& portal) { return portal.cell == cell; });
}

std::pair<int, int> nearest_safe_cell(
        const std::vector<Tile>& grid, int width, int height,
        float target_x, float target_y,
        const std::vector<Portal>& portals,
        const std::vector<ElementInfo>& elements,
        bool exclude_elements) {
    std::optional<std::pair<int, int>> best;
    int best_distance = std::numeric_limits<int>::max();
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const Tile tile = grid[static_cast<size_t>(y * width + x)];
            if (!is_walkable_tile(tile) ||
                contains_portal(portals, {x, y}) ||
                (exclude_elements &&
                 contains_element(elements, {x, y})) ) {
                continue;
            }
            const int distance =
                static_cast<int>(std::lround(std::abs(static_cast<float>(x) + 0.5f - target_x) +
                                             std::abs(static_cast<float>(y) + 0.5f - target_y)));
            if (!best.has_value() || distance < best_distance ||
                (distance == best_distance &&
                 (y < best->second ||
                  (y == best->second && x < best->first)))) {
                best = {x, y};
                best_distance = distance;
            }
        }
    }
    if (!best.has_value()) {
        throw std::runtime_error("no existe una celda caminable segura");
    }
    return *best;
}

}  // namespace

std::shared_ptr<const World> WorldLoader::load(
        const std::string& map_name,
        const std::filesystem::path& maps_directory) {
    std::filesystem::path path = maps_directory / (map_name + ".toml");
    std::error_code ec;
    const std::filesystem::path absolute =
        std::filesystem::absolute(path, ec);
    if (ec) {
        throw map_error(map_name, "no se pudo resolver la ruta");
    }

    toml::table root;
    try {
        root = toml::parse_file(absolute.string());
    } catch (const toml::parse_error& error) {
        throw map_error(map_name, "TOML invalido: " +
                                  std::string(error.description()));
    }

    const std::string root_name = read_string(root, "name", map_name, "raiz");
    if (map_name != "por_defecto" && root_name != map_name) {
        throw map_error(map_name, "campo raiz 'name' no coincide con el nombre del archivo");
    }
    if (map_name == "por_defecto" && root_name != "map_1") {
        throw map_error(map_name, "alias por_defecto requiere name = \"map_1\"");
    }

    const int tile_size = read_int(root, "tile_size", map_name, "raiz");
    const int root_width = read_int(root, "width", map_name, "raiz");
    const int root_height = read_int(root, "height", map_name, "raiz");
    if (tile_size <= 0 || root_width <= 0 || root_height <= 0 ||
        root_width > std::numeric_limits<uint16_t>::max() ||
        root_height > std::numeric_limits<uint16_t>::max()) {
        throw map_error(map_name, "tamano raiz invalido");
    }

    std::map<std::string, bool> solid_layers;
    const toml::array* layers = root["layer"].as_array();
    if (layers == nullptr) {
        throw map_error(map_name, "faltan definiciones [[layer]]");
    }
    for (const toml::node& node : *layers) {
        const toml::table* layer = node.as_table();
        if (layer == nullptr) {
            throw map_error(map_name, "una definicion [[layer]] invalida");
        }
        const std::string name = read_string(*layer, "name", map_name, "[[layer]]");
        if (solid_layers.count(name) != 0) {
            throw map_error(map_name, "layer '" + name + "' declarado mas de una vez");
        }
        const toml::node* solid_node = layer->get("solid");
        if (solid_node == nullptr || !solid_node->is_boolean()) {
            throw map_error(map_name, "layer '" + name + "' falta el campo 'solid' booleano");
        }
        solid_layers.emplace(name, solid_node->value_or(false));
    }

    std::set<std::string> sheets;
    const toml::array* sheet_array = root["sheet"].as_array();
    if (sheet_array != nullptr) {
        for (const toml::node& node : *sheet_array) {
            const toml::table* sheet = node.as_table();
            if (sheet == nullptr) {
                throw map_error(map_name, "una definicion [[sheet]] invalida");
            }
            const std::string path = read_string(*sheet, "path", map_name, "[[sheet]]");
            if (!sheets.insert(path).second) {
                throw map_error(map_name, "sheet '" + path + "' declarado mas de una vez");
            }
            const int tile_w = read_int(*sheet, "tile_w", map_name, "sheet '" + path + "'");
            const int tile_h = read_int(*sheet, "tile_h", map_name, "sheet '" + path + "'");
            if (tile_w <= 0 || tile_h <= 0) {
                throw map_error(map_name, "sheet '" + path +
                                          "' tile_w/tile_h deben ser positivos");
            }
        }
    }

    const toml::array* zones_array = root["zone"].as_array();
    if (zones_array == nullptr) {
        throw map_error(map_name, "faltan definiciones [[zone]]");
    }

    std::map<ZoneId, std::shared_ptr<const Zone>> zones;
    std::map<std::string, ZoneInfo> zone_info;
    std::map<ZoneId, std::vector<Tile>> logical_grids;
    std::map<ZoneId, std::vector<VisualTile>> visual_grids;
    std::map<ZoneId, std::vector<ElementInfo>> element_grids;
    std::map<ZoneId, std::vector<Portal>> zone_portals;
    std::optional<Rectangle> cabin_rect;
    std::optional<Rectangle> center_rect;

    for (const toml::node& zone_node : *zones_array) {
        const toml::table* zone_table = zone_node.as_table();
        if (zone_table == nullptr) {
            throw map_error(map_name, "una definicion [[zone]] invalida");
        }
        const std::string kind = read_string(*zone_table, "kind", map_name, "[[zone]]");
        const int level = read_int(*zone_table, "level", map_name, kind);
        const int width = read_int(*zone_table, "width", map_name, kind);
        const int height = read_int(*zone_table, "height", map_name, kind);
        if (width <= 0 || height <= 0 ||
            width > std::numeric_limits<uint16_t>::max() ||
            height > std::numeric_limits<uint16_t>::max()) {
            throw map_error(map_name, "zona '" + kind + "' dimensiones invalidas");
        }

        ZoneId id;
        if (kind == "farm") {
            if (level != 0) throw map_error(map_name, "zona 'farm' debe tener nivel 0");
            id = ZoneId::FARM_ZONE;
            cabin_rect = read_rect(*zone_table, "cabin_rect", map_name, kind, width, height);
        } else if (kind == "village") {
            if (level != 0) throw map_error(map_name, "zona 'village' debe tener nivel 0");
            id = ZoneId::VILLAGE_ZONE;
            center_rect = read_rect(*zone_table, "community_center_rect", map_name, kind,
                                    width, height);
        } else if (kind == "house") {
            if (level != 0) throw map_error(map_name, "zona 'house' debe tener nivel 0");
            id = ZoneId::HOUSE_ZONE;
        } else if (kind == "community_center") {
            if (level != 0) throw map_error(map_name, "zona 'community_center' debe tener nivel 0");
            id = ZoneId::COMMUNITY_CENTER_ZONE;
        } else if (kind == "cave") {
            if (level < 1 || level > 10) {
                throw map_error(map_name, "zona 'cave' nivel fuera de 1-10");
            }
            id = {ZoneId::Kind::CAVE_LEVEL, level};
        } else {
            throw map_error(map_name, "zona desconocida '" + kind + "'");
        }
        if (zones.count(id) != 0) {
            throw map_error(map_name, "zona duplicada '" + kind +
                                      (kind == "cave" ? " nivel " + std::to_string(level) : ""));
        }

        ZoneInfo info{kind, level, width, height, {}};
        const toml::node* elements_node = zone_table->get("element");
        const toml::array* elements =
            elements_node == nullptr ? nullptr : elements_node->as_array();
        if (elements != nullptr) {
            for (const toml::node& element_node : *elements) {
                const toml::table* element = element_node.as_table();
                if (element == nullptr) {
                    throw map_error(map_name, "elemento invalido en zona '" + kind + "'");
                }
                ElementInfo item;
                item.type = read_string(*element, "type", map_name,
                                        "elemento en zona '" + kind + "'");
                item.x = read_int(*element, "x", map_name, kind);
                item.y = read_int(*element, "y", map_name, kind);
                if (item.x < 0 || item.y < 0 || item.x >= width || item.y >= height) {
                    throw map_error(map_name, "elemento '" + item.type +
                                              "' fuera de limites en zona '" + kind + "'");
                }
                info.elements.push_back(item);
            }
        }
        element_grids[id] = info.elements;

        std::vector<Tile> grid(static_cast<size_t>(width) * height, Tile::WALL);
        std::set<std::pair<std::pair<int, int>, std::string>> tile_keys;
        std::vector<VisualTile> visual;
        const toml::node* tiles_node = zone_table->get("tile");
        const toml::array* tiles =
            tiles_node == nullptr ? nullptr : tiles_node->as_array();
        if (tiles != nullptr) {
            for (const toml::node& tile_node : *tiles) {
                const toml::table* tile = tile_node.as_table();
                if (tile == nullptr) {
                    throw map_error(map_name, "tile invalido en zona '" + kind + "'");
                }
                const int x = read_int(*tile, "x", map_name, kind);
                const int y = read_int(*tile, "y", map_name, kind);
                const int col = read_int(*tile, "col", map_name, kind);
                const int row = read_int(*tile, "row", map_name, kind);
                const std::string sheet = read_string(*tile, "sheet", map_name,
                                                      "tile en zona '" + kind + "'");
                const std::string layer = read_string(*tile, "layer", map_name,
                                                      "tile en zona '" + kind + "'");
                if (x < 0 || y < 0 || x >= width || y >= height ||
                    col < 0 || row < 0) {
                    throw map_error(map_name, "tile invalido en zona '" + kind +
                                              "' coordenadas negativas o fuera de limite");
                }
                if (!sheets.count(sheet) && sheets.size() != 0) {
                    throw map_error(map_name, "zona '" + kind + "' referencia sheet '" +
                                              sheet + "' no declarada");
                }
                if (sheet.empty()) {
                    throw map_error(map_name, "zona '" + kind + "' referencia sheet vacia");
                }
                if (sheets.empty()) {
                    throw map_error(map_name, "zona '" + kind +
                                              "' referencia sheet sin [[sheet]] declarado");
                }
                const auto layer_found = solid_layers.find(layer);
                if (layer_found == solid_layers.end()) {
                    throw map_error(map_name, "zona '" + kind + "' referencia layer '" +
                                              layer + "' no declarada");
                }
                if (!tile_keys.insert({{x, y}, layer}).second) {
                    throw map_error(map_name, "zona '" + kind + "' tile duplicado en (" +
                                              std::to_string(x) + "," + std::to_string(y) +
                                              ") capa '" + layer + "'");
                }
                Tile& logical = grid[static_cast<size_t>(y * width + x)];
                if (layer_found->second) {
                    logical = Tile::WALL;
                } else if (layer == "ground") {
                    logical = Tile::FLOOR;
                }
                visual.push_back({x, y, layer, sheet, col, row});
            }
        }

        zone_info[kind == "cave" ? kind + std::to_string(level) : kind] = info;
        logical_grids[id] = std::move(grid);
        visual_grids[id] = std::move(visual);
    }

    for (const std::string kind : {"farm", "village", "house", "community_center"}) {
        bool found = false;
        for (const auto& [id, info] : zone_info) {
            if (info.kind == kind) found = true;
        }
        if (!found) throw map_error(map_name, "falta zona requerida '" + kind + "'");
    }
    for (int level = 1; level <= 10; ++level) {
        if (!zone_info.count("cave" + std::to_string(level))) {
            throw map_error(map_name, "falta zona cave nivel " + std::to_string(level));
        }
    }

    // Build portals and validate unique element semantics.
    auto& farm_grid = logical_grids[ZoneId::FARM_ZONE];
    auto& village_grid = logical_grids[ZoneId::VILLAGE_ZONE];
    auto& house_grid = logical_grids[ZoneId::HOUSE_ZONE];
    auto& center_grid = logical_grids[ZoneId::COMMUNITY_CENTER_ZONE];
    const ZoneInfo& farm_info = zone_info["farm"];
    const ZoneInfo& village_info = zone_info["village"];
    const ZoneInfo& house_info = zone_info["house"];
    const ZoneInfo& center_info = zone_info["community_center"];

    auto elements_of = [&](ZoneId id) -> std::vector<ElementInfo>& {
        return element_grids[id];
    };
    auto count_type = [](const std::vector<ElementInfo>& elements,
                         const std::string& type) {
        return static_cast<int>(std::count_if(
            elements.begin(), elements.end(),
            [&](const ElementInfo& element) { return element.type == type; }));
    };

    if (count_type(elements_of(ZoneId::FARM_ZONE), "house_door") != 1 ||
        count_type(elements_of(ZoneId::FARM_ZONE), "road") != 1 ||
        count_type(elements_of(ZoneId::HOUSE_ZONE), "house_exit") != 1 ||
        count_type(elements_of(ZoneId::HOUSE_ZONE), "bed") != 1 ||
        count_type(elements_of(ZoneId::VILLAGE_ZONE), "center_door") != 1 ||
        count_type(elements_of(ZoneId::VILLAGE_ZONE), "road") != 1 ||
        count_type(elements_of(ZoneId::COMMUNITY_CENTER_ZONE), "center_exit") != 1) {
        throw map_error(map_name, "faltan o se repiten portales/interiores requeridos");
    }

    for (const std::string merchant : {"pierre", "marnie", "clint", "robin"}) {
        if (count_type(elements_of(ZoneId::VILLAGE_ZONE), merchant) != 1) {
            throw map_error(map_name, "comerciante '" + merchant +
                                      "' debe aparecer exactamente una vez en village");
        }
    }

    std::set<std::pair<int, int>> all_element_cells;
    for (const auto& [id, elements] : element_grids) {
        std::set<std::pair<int, int>> zone_cells;
        for (const ElementInfo& element : elements) {
            bool allowed = false;
            if (id == ZoneId::FARM_ZONE) {
                allowed = element.type == "road" ||
                          element.type == "dungeon_door" ||
                          element.type == "house_door";
            } else if (id == ZoneId::VILLAGE_ZONE) {
                allowed = element.type == "road" ||
                          element.type == "dungeon_door" ||
                          element.type == "center_door" ||
                          element.type == "pierre" ||
                          element.type == "marnie" ||
                          element.type == "clint" ||
                          element.type == "robin";
            } else if (id == ZoneId::HOUSE_ZONE) {
                allowed = element.type == "bed" ||
                          element.type == "house_exit";
            } else if (id == ZoneId::COMMUNITY_CENTER_ZONE) {
                allowed = element.type == "center_exit";
            } else {
                allowed = element.type == "dungeon_exit" ||
                          element.type == "stair_down" ||
                          element.type == "stair_up" ||
                          element.type == "enemy" ||
                          element.type == "ore_coal" ||
                          element.type == "ore_lapis" ||
                          element.type == "ore_silver" ||
                          element.type == "ore_gold" ||
                          element.type == "trap_spikes" ||
                          element.type == "trap_flame_left" ||
                          element.type == "trap_flame_right" ||
                          element.type == "trap_flame_down";
            }
            if (!allowed) {
                throw map_error(map_name, "elemento desconocido o no permitido '" +
                                          element.type + "' en zona");
            }
            if (!zone_cells.insert({element.x, element.y}).second) {
                const std::string zone_name =
                        id.kind == ZoneId::Kind::CAVE_LEVEL
                        ? "cave" + std::to_string(id.level)
                        : id.kind == ZoneId::Kind::FARM ? std::string("farm")
                        : id.kind == ZoneId::Kind::VILLAGE ? std::string("village")
                        : id.kind == ZoneId::Kind::HOUSE ? std::string("house")
                        : std::string("community_center");
                        throw map_error(map_name, "dos elementos en la misma celda en zona '" + zone_name + "'");
            }
        }
    }

    auto find_element = [&](ZoneId id, const std::string& type) {
        const auto& elements = elements_of(id);
        auto found = std::find_if(elements.begin(), elements.end(),
                                  [&](const ElementInfo& element) {
                                      return element.type == type;
                                  });
        if (found == elements.end()) {
            throw map_error(map_name, "falta elemento '" + type + "' en mapa");
        }
        return std::pair<int, int>{found->x, found->y};
    };

    const auto house_door = find_element(ZoneId::FARM_ZONE, "house_door");
    const auto house_exit = find_element(ZoneId::HOUSE_ZONE, "house_exit");
    const auto center_door = find_element(ZoneId::VILLAGE_ZONE, "center_door");
    const auto center_exit = find_element(ZoneId::COMMUNITY_CENTER_ZONE, "center_exit");
    const auto farm_road = find_element(ZoneId::FARM_ZONE, "road");
    const auto village_road = find_element(ZoneId::VILLAGE_ZONE, "road");
    const auto bed = find_element(ZoneId::HOUSE_ZONE, "bed");

    if (!cabin_rect->contains_cell(house_door.first, house_door.second)) {
        throw map_error(map_name, "house_door debe estar dentro de cabin_rect");
    }
    if (!center_rect->contains_cell(center_door.first, center_door.second)) {
        throw map_error(map_name, "center_door debe estar dentro de community_center_rect");
    }

    std::vector<Portal> house_portals{
        {PortalType::HOUSE_EXIT, house_exit, ZoneId::FARM_ZONE, center(house_door)}};
    // Correct house exit arrival and house door arrival.
    std::vector<Portal> farm_portals{
        {PortalType::HOUSE_DOOR, house_door, ZoneId::HOUSE_ZONE, center(house_exit)},
        {PortalType::ROAD, farm_road, ZoneId::VILLAGE_ZONE, center(village_road)}};
    std::vector<Portal> village_portals{
        {PortalType::CENTER_DOOR, center_door, ZoneId::COMMUNITY_CENTER_ZONE, center(center_exit)},
        {PortalType::ROAD, village_road, ZoneId::FARM_ZONE, center(farm_road)}};
    std::vector<Portal> center_portals{
        {PortalType::CENTER_EXIT, center_exit, ZoneId::VILLAGE_ZONE, center(center_door)}};

    for (Portal& portal : farm_portals) {
        if (!is_walkable_tile(farm_grid[static_cast<size_t>(
                                 portal.cell.second * farm_info.width +
                                 portal.cell.first)])) {
            farm_grid[static_cast<size_t>(portal.cell.second * farm_info.width +
                                          portal.cell.first)] = Tile::FLOOR;
        }
    }
    for (Portal& portal : village_portals) {
        if (!is_walkable_tile(village_grid[static_cast<size_t>(
                                 portal.cell.second * village_info.width +
                                 portal.cell.first)])) {
            village_grid[static_cast<size_t>(portal.cell.second * village_info.width +
                                             portal.cell.first)] = Tile::FLOOR;
        }
    }
    for (Portal& portal : house_portals) {
        house_grid[static_cast<size_t>(portal.cell.second * house_info.width +
                                       portal.cell.first)] = Tile::FLOOR;
    }
    for (Portal& portal : center_portals) {
        center_grid[static_cast<size_t>(portal.cell.second * center_info.width +
                                        portal.cell.first)] = Tile::FLOOR;
    }

    // Dungeon door must be exactly once in farm or village.
    int dungeon_doors = count_type(elements_of(ZoneId::FARM_ZONE), "dungeon_door") +
                        count_type(elements_of(ZoneId::VILLAGE_ZONE), "dungeon_door");
    if (dungeon_doors != 1) {
        throw map_error(map_name, "debe existir exactamente una dungeon_door");
    }
    ZoneId dungeon_surface =
        count_type(elements_of(ZoneId::FARM_ZONE), "dungeon_door") == 1
            ? ZoneId::FARM_ZONE : ZoneId::VILLAGE_ZONE;
    const auto dungeon_door = dungeon_surface == ZoneId::FARM_ZONE
                                  ? find_element(ZoneId::FARM_ZONE, "dungeon_door")
                                  : find_element(ZoneId::VILLAGE_ZONE, "dungeon_door");
    if (dungeon_surface == ZoneId::FARM_ZONE) {
        farm_portals.push_back({PortalType::DUNGEON_DOOR, dungeon_door,
                                {ZoneId::Kind::CAVE_LEVEL, 1}, {1.5f, 1.5f}});
    } else {
        village_portals.push_back({PortalType::DUNGEON_DOOR, dungeon_door,
                                   {ZoneId::Kind::CAVE_LEVEL, 1}, {1.5f, 1.5f}});
    }

    std::map<int, std::vector<Portal>> cave_portals;
    std::optional<std::pair<int, int>> safe_surface;
    for (int level = 1; level <= 10; ++level) {
        const ZoneId id{ZoneId::Kind::CAVE_LEVEL, level};
        auto& elements = elements_of(id);
        if (count_type(elements, "dungeon_exit") != 1) {
            throw map_error(map_name, "cave nivel " + std::to_string(level) +
                                      " debe tener exactamente un dungeon_exit");
        }
        const int down_count = count_type(elements, "stair_down");
        const int up_count = count_type(elements, "stair_up");
        if (level == 1 && (down_count != 1 || up_count != 0)) {
            throw map_error(map_name, "cave nivel 1 requiere stair_down y no stair_up");
        } else if (level == 10 && (up_count != 1 || down_count != 0)) {
            throw map_error(map_name, "cave nivel 10 requiere stair_up y no stair_down");
        } else if (level >= 2 && level <= 9 && (down_count != 1 || up_count != 1)) {
            throw map_error(map_name, "cave nivel " + std::to_string(level) +
                                      " requiere stair_down y stair_up");
        }

        for (const ElementInfo& element : elements) {
            const bool cave_only =
                element.type == "enemy" || element.type.rfind("ore_", 0) == 0 ||
                element.type.rfind("trap_", 0) == 0 ||
                element.type == "dungeon_exit" ||
                element.type == "stair_down" || element.type == "stair_up";
            if (!cave_only) {
                throw map_error(map_name, "elemento '" + element.type +
                                          "' no permitido en cave nivel " +
                                          std::to_string(level));
            }
            if (element.type == "ore_gold" && level < 5) {
                throw map_error(map_name, "ore_gold no puede aparecer antes del nivel 5");
            }
        }

        const auto dungeon_exit = find_element(id, "dungeon_exit");
        cave_portals[level].push_back(
            {PortalType::DUNGEON_EXIT, dungeon_exit, dungeon_surface, {0.0f, 0.0f}});
    }

    // Choose one safe surface return cell for every cave exit.
    auto& surface_grid = dungeon_surface == ZoneId::FARM_ZONE
                             ? logical_grids[ZoneId::FARM_ZONE]
                             : logical_grids[ZoneId::VILLAGE_ZONE];
    const ZoneInfo& surface_info = dungeon_surface == ZoneId::FARM_ZONE
                                       ? farm_info : village_info;
    const std::vector<Portal>& surface_portals =
        dungeon_surface == ZoneId::FARM_ZONE ? farm_portals : village_portals;
    safe_surface = nearest_safe_cell(surface_grid, surface_info.width, surface_info.height,
                                     static_cast<float>(dungeon_door.first) + 0.5f,
                                     static_cast<float>(dungeon_door.second) + 0.5f,
                                     surface_portals,
                                     elements_of(dungeon_surface),
                                     false);
    const auto safe_surface_arrival = center(*safe_surface);

    // Portal openings and cave stair reciprocity.
    for (int level = 1; level <= 10; ++level) {
        const ZoneId id{ZoneId::Kind::CAVE_LEVEL, level};
        const ZoneInfo& cave_info = zone_info["cave" + std::to_string(level)];
        auto& grid = logical_grids[id];
        auto& portals = cave_portals[level];
        portals.front().arrival = safe_surface_arrival;
        const auto& elements = elements_of(id);
        for (const ElementInfo& element : elements) {
            if (element.type == "dungeon_exit" ||
                element.type == "stair_down" || element.type == "stair_up") {
                const bool walkable = is_walkable_tile(
                    grid[static_cast<size_t>(element.y * cave_info.width + element.x)]);
                if (!walkable) {
                    grid[static_cast<size_t>(element.y * cave_info.width + element.x)] =
                        Tile::FLOOR;
                }
            }
        }
        if (level < 10) {
            const auto down = find_element(id, "stair_down");
            const ZoneId next{ZoneId::Kind::CAVE_LEVEL, level + 1};
            const auto up = find_element(next, "stair_up");
            portals.push_back({PortalType::STAIR, down, next, center(up)});
        }
        if (level > 1) {
            const auto up = find_element(id, "stair_up");
            const ZoneId previous{ZoneId::Kind::CAVE_LEVEL, level - 1};
            const auto down = find_element(previous, "stair_down");
            portals.push_back({PortalType::STAIR, up, previous, center(down)});
        }

        for (const ElementInfo& element : elements) {
            if (element.type == "enemy" || element.type.rfind("ore_", 0) == 0 ||
                element.type.rfind("trap_", 0) == 0) {
                if (!is_walkable_tile(
                        grid[static_cast<size_t>(element.y * cave_info.width +
                                                 element.x)])) {
                    throw map_error(map_name, "elemento '" + element.type +
                                              "' no esta sobre una celda caminable");
                }
                if (contains_portal(portals,
                                    {element.x, element.y})) {
                    throw map_error(map_name, "elemento '" + element.type +
                                              "' no puede estar sobre un portal");
                }
            }
        }
    }

    // Deterministic cave 1 entry arrival.
    const ZoneId cave1{ZoneId::Kind::CAVE_LEVEL, 1};
    const ZoneInfo& cave1_info = zone_info["cave1"];
    const auto cave1_safe = nearest_safe_cell(
        logical_grids[cave1], cave1_info.width, cave1_info.height,
        static_cast<float>(cave1_info.width) / 2.0f,
        static_cast<float>(cave1_info.height) / 2.0f,
        cave_portals[1], elements_of(cave1), true);
    const auto cave1_arrival = center(cave1_safe);
    if (dungeon_surface == ZoneId::FARM_ZONE) {
        farm_portals.back().arrival = cave1_arrival;
    } else {
        village_portals.back().arrival = cave1_arrival;
    }

    // Construct zones.
    for (const auto& [id, grid] : logical_grids) {
        const ZoneInfo& info = zone_info[
            id.kind == ZoneId::Kind::CAVE_LEVEL ? "cave" + std::to_string(id.level)
            : id.kind == ZoneId::Kind::FARM ? "farm"
            : id.kind == ZoneId::Kind::VILLAGE ? "village"
            : id.kind == ZoneId::Kind::HOUSE ? "house"
            : "community_center"];
        std::vector<Portal> portals;
        if (id == ZoneId::FARM_ZONE) portals = farm_portals;
        else if (id == ZoneId::VILLAGE_ZONE) portals = village_portals;
        else if (id == ZoneId::HOUSE_ZONE) portals = house_portals;
        else if (id == ZoneId::COMMUNITY_CENTER_ZONE) portals = center_portals;
        else portals = cave_portals[id.level];

        std::shared_ptr<Zone> zone;
        if (id == ZoneId::FARM_ZONE) {
            zone = std::make_shared<FarmZone>(
                info.width, info.height, grid, *cabin_rect,
                std::vector<std::pair<float, float>>{}, portals);
        } else if (id == ZoneId::VILLAGE_ZONE) {
            std::vector<MerchantPosition> merchants;
            for (const ElementInfo& element : elements_of(id)) {
                MerchantType type;
                if (element.type == "pierre") type = MerchantType::PIERRE;
                else if (element.type == "marnie") type = MerchantType::MARNIE;
                else if (element.type == "clint") type = MerchantType::CLINT;
                else if (element.type == "robin") type = MerchantType::ROBIN;
                else continue;
                merchants.push_back({type, static_cast<float>(element.x) + 0.5f,
                                     static_cast<float>(element.y) + 0.5f});
            }
            zone = std::make_shared<VillageZone>(
                info.width, info.height, grid, portals,
                std::vector<ZoneId::ZoneArea>{}, merchants, center_rect);
        } else if (id.kind == ZoneId::Kind::CAVE_LEVEL) {
            std::vector<std::pair<float, float>> spawns;
            for (const ElementInfo& element : elements_of(id)) {
                if (element.type == "enemy") {
                    spawns.emplace_back(static_cast<float>(element.x) + 0.5f,
                                        static_cast<float>(element.y) + 0.5f);
                }
            }
            if (spawns.empty()) {
                throw map_error(map_name, "cave nivel " + std::to_string(id.level) +
                                          " debe tener al menos un enemy");
            }
            zone = std::make_shared<CaveLevel>(
                id.level, info.width, info.height, grid, portals, spawns);
        } else {
            std::vector<std::pair<float, float>> beds;
            if (id == ZoneId::HOUSE_ZONE) {
                beds.emplace_back(static_cast<float>(bed.first) + 0.5f,
                                  static_cast<float>(bed.second) + 0.5f);
            }
            zone = std::make_shared<InteriorZone>(id, info.width, info.height,
                                                  grid, portals, beds);
        }

        std::vector<StaticElement> static_elements;
        for (const ElementInfo& element : elements_of(id)) {
            StaticElementType type;
            if (element.type == "bed") type = StaticElementType::BED;
            else if (element.type == "pierre") type = StaticElementType::PIERRE;
            else if (element.type == "marnie") type = StaticElementType::MARNIE;
            else if (element.type == "clint") type = StaticElementType::CLINT;
            else if (element.type == "robin") type = StaticElementType::ROBIN;
            else if (element.type == "enemy") type = StaticElementType::ENEMY_SPAWN;
            else if (element.type == "ore_coal") type = StaticElementType::ORE_COAL;
            else if (element.type == "ore_lapis") type = StaticElementType::ORE_LAPIS;
            else if (element.type == "ore_silver") type = StaticElementType::ORE_SILVER;
            else if (element.type == "ore_gold") type = StaticElementType::ORE_GOLD;
            else if (element.type == "trap_spikes") type = StaticElementType::TRAP_SPIKES;
            else if (element.type == "trap_flame_left") type = StaticElementType::TRAP_FLAME_LEFT;
            else if (element.type == "trap_flame_right") type = StaticElementType::TRAP_FLAME_RIGHT;
            else if (element.type == "trap_flame_down") type = StaticElementType::TRAP_FLAME_DOWN;
            else continue;
            static_elements.push_back({type, {element.x, element.y}});
        }
        zone->set_static_elements(std::move(static_elements));
        zone->set_visual_tiles(visual_grids[id]);
        zones.emplace(id, zone);
    }

    return std::make_shared<const World>(std::move(zones));
}

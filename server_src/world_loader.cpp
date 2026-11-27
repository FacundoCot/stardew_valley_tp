#include "world_loader.h"

#include <toml++/toml.hpp>

#include <algorithm>
#include <filesystem>
#include <limits>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include <array>

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

Rectangle infer_structure_rect(
        const std::set<std::pair<int, int>>& solid_cells,
        const std::pair<int, int>& portal, const std::string& map_name,
        const std::string& zone_name, const std::string& structure_name,
        int width, int height) {
    std::vector<std::pair<int, int>> pending;
    std::set<std::pair<int, int>> component;
    constexpr std::array<std::pair<int, int>, 4> neighbors{{
        {1, 0}, {-1, 0}, {0, 1}, {0, -1}}};

    if (solid_cells.count(portal) != 0) {
        pending.push_back(portal);
        component.insert(portal);
    }

    for (const auto& [dx, dy] : neighbors) {
        const std::pair<int, int> cell{portal.first + dx, portal.second + dy};
        if (solid_cells.count(cell) != 0 && component.insert(cell).second) {
            pending.push_back(cell);
        }
    }

    for (size_t next = 0; next < pending.size(); ++next) {
        const auto [x, y] = pending[next];

        for (const auto& [dx, dy] : neighbors) {
            const std::pair<int, int> cell{x + dx, y + dy};
            if (solid_cells.count(cell) != 0 && component.insert(cell).second) {
                pending.push_back(cell);
            }
        }
    }

    if (component.empty()) {
        throw map_error(map_name, "no se pudo inferir " + structure_name +
                                  " en zona '" + zone_name +
                                  "': el portal no tiene paredes contiguas");
    }

    int min_x = portal.first;
    int max_x = portal.first;
    int min_y = portal.second;
    int max_y = portal.second;

    for (const auto& [x, y] : component) {
        min_x = std::min(min_x, x);
        max_x = std::max(max_x, x);
        min_y = std::min(min_y, y);
        max_y = std::max(max_y, y);
    }

    const Rectangle rectangle{
        min_x, min_y, max_x - min_x + 1, max_y - min_y + 1};
    if (rectangle.x < 0 || rectangle.y < 0 ||
        rectangle.x + rectangle.width > width ||
        rectangle.y + rectangle.height > height ||
        !rectangle.contains_cell(portal.first, portal.second)) {
        throw map_error(map_name, "rectangulo inferido de " + structure_name +
                                  " invalido en zona '" + zone_name + "'");
    }
    return rectangle;
}

std::pair<float, float> center(const std::pair<int, int>& cell) {
    return {static_cast<float>(cell.first) + 0.5f,
            static_cast<float>(cell.second) + 0.5f};
}

bool contains_portal(const std::vector<Portal>& portals,
                     const std::pair<int, int>& cell) {
    return std::any_of(portals.begin(), portals.end(),
                       [&](const Portal& portal) { return portal.cell == cell; });
}

bool is_floor_element(const std::string& type) {
    return type == "road" || type == "dungeon_door" ||
           type == "house_door" || type == "center_door" ||
           type == "stair_down" || type == "stair_up" ||
           type == "dungeon_exit" || type == "house_exit" ||
           type == "center_exit" || type == "enemy" ||
           type == "marnie" || type == "clint" ||
           type == "pierre" || type == "robin" ||
           type == "trap_spikes" || type == "offering_crops" ||
           type == "offering_metals" || type == "offering_animals" ||
           type == "offering_fight" || type == "offering_money";
}

bool is_portal_element(const std::string& type) {
    return type == "road" || type == "dungeon_door" ||
           type == "house_door" || type == "center_door" ||
           type == "stair_down" || type == "stair_up" ||
           type == "dungeon_exit" || type == "house_exit" ||
           type == "center_exit";
}

std::pair<float, float> portal_arrival(
        const std::vector<Tile>& grid, int width, int height,
        const std::pair<int, int>& destination,
        const std::vector<Portal>& destination_portals,
        const std::set<std::pair<int, int>>& solid_cells) {
    constexpr std::array<std::pair<int, int>, 4> directions{{
        {1, 0}, {-1, 0}, {0, 1}, {0, -1}}};
    std::vector<std::pair<int, int>> candidates;
    for (const auto& [dx, dy] : directions) {
        if (solid_cells.count({destination.first + dx,
                               destination.second + dy}) != 0) {
            candidates.emplace_back(destination.first - dx,
                                    destination.second - dy);
        }
    }
    for (const auto& [dx, dy] : directions) {
        candidates.emplace_back(destination.first + dx,
                                destination.second + dy);
    }

    std::set<std::pair<int, int>> considered;
    for (const auto& candidate : candidates) {
        const auto [x, y] = candidate;
        if (!considered.insert(candidate).second || x < 0 || y < 0 ||
            x >= width || y >= height ||
            !is_walkable_tile(grid[static_cast<size_t>(y * width + x)]) ||
            contains_portal(destination_portals, candidate)) {
            continue;
        }
        return center(candidate);
    }
    throw std::runtime_error(
        "no existe un vecino caminable para la llegada del portal");
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
    std::map<ZoneId, std::set<std::pair<int, int>>> solid_cells;

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
        } else if (kind == "village") {
            if (level != 0) throw map_error(map_name, "zona 'village' debe tener nivel 0");
            id = ZoneId::VILLAGE_ZONE;
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
        std::vector<bool> has_solid_tile(grid.size(), false);
        std::vector<bool> has_non_solid_tile(grid.size(), false);
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
                const size_t cell_index = static_cast<size_t>(y * width + x);
                if (layer_found->second) {
                    has_solid_tile[cell_index] = true;
                    if (layer == "walls" || layer == "objects") {
                        solid_cells[id].insert({x, y});
                    }
                } else {
                    has_non_solid_tile[cell_index] = true;
                }
                visual.push_back({x, y, layer, sheet, col, row});
            }
        }
        for (size_t cell_index = 0; cell_index < grid.size(); ++cell_index) {
            if (has_solid_tile[cell_index]) {
                grid[cell_index] = Tile::WALL;
            } else if (has_non_solid_tile[cell_index]) {
                grid[cell_index] = Tile::FLOOR;
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
    const ZoneInfo& farm_info = zone_info["farm"];
    const ZoneInfo& village_info = zone_info["village"];
    

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
                allowed = element.type == "center_exit" ||
                          element.type == "offering_crops" ||
                          element.type == "offering_metals" ||
                          element.type == "offering_animals" ||
                          element.type == "offering_fight" ||
                          element.type == "offering_money";
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
                          element.type == "trap_flame_down" ||
                          element.type == "trap_flame_up" ||
                          element.type == "abandoned_chest";
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

    const Rectangle cabin_rect = infer_structure_rect(
        solid_cells[ZoneId::FARM_ZONE], house_door,
        map_name, "farm", "la cabania", farm_info.width, farm_info.height);

    const Rectangle center_rect = infer_structure_rect(
        solid_cells[ZoneId::VILLAGE_ZONE], center_door,
        map_name, "village", "el centro comunitario",
        village_info.width, village_info.height);

    std::vector<Portal> house_portals{
        {PortalType::HOUSE_EXIT, house_exit, ZoneId::FARM_ZONE, {0.0f, 0.0f}}};
    std::vector<Portal> farm_portals{
        {PortalType::HOUSE_DOOR, house_door, ZoneId::HOUSE_ZONE, {0.0f, 0.0f}},
        {PortalType::ROAD, farm_road, ZoneId::VILLAGE_ZONE, {0.0f, 0.0f}}};
    std::vector<Portal> village_portals{
        {PortalType::CENTER_DOOR, center_door, ZoneId::COMMUNITY_CENTER_ZONE, {0.0f, 0.0f}},
        {PortalType::ROAD, village_road, ZoneId::FARM_ZONE, {0.0f, 0.0f}}};
    std::vector<Portal> center_portals{
        {PortalType::CENTER_EXIT, center_exit, ZoneId::VILLAGE_ZONE, {0.0f, 0.0f}}};

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
                element.type == "trap_spikes" ||
                element.type == "trap_flame_up" ||
                element.type == "trap_flame_down" ||
                element.type == "trap_flame_left" ||
                element.type == "trap_flame_right" ||
                element.type == "abandoned_chest" ||
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
        if (level < 10) {
            const auto down = find_element(id, "stair_down");
            const ZoneId next{ZoneId::Kind::CAVE_LEVEL, level + 1};
            find_element(next, "stair_up");
            cave_portals[level].push_back(
                {PortalType::STAIR, down, next, {0.0f, 0.0f}});
        }
        if (level > 1) {
            const auto up = find_element(id, "stair_up");
            const ZoneId previous{ZoneId::Kind::CAVE_LEVEL, level - 1};
            find_element(previous, "stair_down");
            cave_portals[level].push_back(
                {PortalType::STAIR, up, previous, {0.0f, 0.0f}});
        }
    }

    auto portals_for = [&](ZoneId id) -> const std::vector<Portal>& {
        if (id == ZoneId::FARM_ZONE) return farm_portals;
        if (id == ZoneId::VILLAGE_ZONE) return village_portals;
        if (id == ZoneId::HOUSE_ZONE) return house_portals;
        if (id == ZoneId::COMMUNITY_CENTER_ZONE) return center_portals;
        return cave_portals.at(id.level);
    };

    for (const auto& entry : logical_grids) {
        const ZoneId& id = entry.first;
        const ZoneInfo& info = zone_info[
            id.kind == ZoneId::Kind::CAVE_LEVEL
                ? "cave" + std::to_string(id.level)
                : id.kind == ZoneId::Kind::FARM ? "farm"
                : id.kind == ZoneId::Kind::VILLAGE ? "village"
                : id.kind == ZoneId::Kind::HOUSE ? "house"
                : "community_center"];
        auto& mutable_grid = logical_grids[id];
        for (const Portal& portal : portals_for(id)) {
            mutable_grid[static_cast<size_t>(
                portal.cell.second * info.width + portal.cell.first)] =
                Tile::FLOOR;
        }
    }

    for (const auto& [id, elements] : element_grids) {
        const ZoneInfo& info = zone_info[
            id.kind == ZoneId::Kind::CAVE_LEVEL
                ? "cave" + std::to_string(id.level)
                : id.kind == ZoneId::Kind::FARM ? "farm"
                : id.kind == ZoneId::Kind::VILLAGE ? "village"
                : id.kind == ZoneId::Kind::HOUSE ? "house"
                : "community_center"];
        const auto& grid = logical_grids[id];
        const auto& portals = portals_for(id);
        for (const ElementInfo& element : elements) {
            const std::pair<int, int> cell{element.x, element.y};
            if (is_floor_element(element.type) &&
                !is_walkable_tile(
                    grid[static_cast<size_t>(element.y * info.width +
                                             element.x)])) {
                throw map_error(map_name, "elemento '" + element.type +
                                          "' no esta sobre una celda caminable");
            }
            if (!is_portal_element(element.type) &&
                contains_portal(portals, cell)) {
                throw map_error(map_name, "elemento '" + element.type +
                                          "' no puede estar sobre un portal");
            }
        }
    }

    auto arrival_at = [&](ZoneId id, const std::pair<int, int>& destination) {
        const ZoneInfo& info = zone_info[
            id.kind == ZoneId::Kind::CAVE_LEVEL
                ? "cave" + std::to_string(id.level)
                : id.kind == ZoneId::Kind::FARM ? "farm"
                : id.kind == ZoneId::Kind::VILLAGE ? "village"
                : id.kind == ZoneId::Kind::HOUSE ? "house"
                : "community_center"];
        return portal_arrival(logical_grids[id], info.width, info.height,
                              destination, portals_for(id), solid_cells[id]);
    };

    house_portals.front().arrival =
        arrival_at(ZoneId::FARM_ZONE, house_door);
    farm_portals[0].arrival = arrival_at(ZoneId::HOUSE_ZONE, house_exit);
    farm_portals[1].arrival = arrival_at(ZoneId::VILLAGE_ZONE, village_road);
    village_portals[0].arrival =
        arrival_at(ZoneId::COMMUNITY_CENTER_ZONE, center_exit);
    village_portals[1].arrival = arrival_at(ZoneId::FARM_ZONE, farm_road);
    center_portals.front().arrival =
        arrival_at(ZoneId::VILLAGE_ZONE, center_door);

    const ZoneId cave1{ZoneId::Kind::CAVE_LEVEL, 1};
    const auto cave1_exit = find_element(cave1, "dungeon_exit");
    const auto cave1_arrival = arrival_at(cave1, cave1_exit);
    auto& surface_portals = dungeon_surface == ZoneId::FARM_ZONE
                                ? farm_portals : village_portals;
    for (Portal& portal : surface_portals) {
        if (portal.type == PortalType::DUNGEON_DOOR) {
            portal.arrival = cave1_arrival;
        }
    }

    const auto surface_arrival = arrival_at(dungeon_surface, dungeon_door);
     for (int level = 1; level <= 10; ++level) {
        for (Portal& portal : cave_portals[level]) {
            if (portal.type == PortalType::DUNGEON_EXIT) {
                portal.arrival = surface_arrival;
                continue;
            }
            const std::string destination_stair =
                portal.destination.level > level ? "stair_up" : "stair_down";
            const auto stair = find_element(portal.destination, destination_stair);
            portal.arrival = arrival_at(portal.destination, stair);
        }
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
                info.width, info.height, grid, cabin_rect,
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
            else if (element.type == "trap_flame_up") type = StaticElementType::TRAP_FLAME_UP;
            else if (element.type == "abandoned_chest") type = StaticElementType::ABANDONED_CHEST;
            else if (element.type == "offering_crops") type = StaticElementType::OFFERING_CROPS;
            else if (element.type == "offering_metals") type = StaticElementType::OFFERING_METALS;
            else if (element.type == "offering_animals") type = StaticElementType::OFFERING_ANIMALS;
            else if (element.type == "offering_fight") type = StaticElementType::OFFERING_FIGHT;
            else if (element.type == "offering_money") type = StaticElementType::OFFERING_MONEY;
            else continue;
            static_elements.push_back({type, {element.x, element.y}});
        }
        zone->set_static_elements(std::move(static_elements));
        zone->set_visual_tiles(visual_grids[id]);
        zones.emplace(id, zone);
    }

    return std::make_shared<const World>(std::move(zones));
}

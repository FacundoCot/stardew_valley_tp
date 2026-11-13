#include "world.h"
#include "define.h"

#include <cmath>
#include <cstdlib>
#include <map>
#include <memory>
#include <optional>
#include <queue>
#include <set>
#include <stdexcept>
#include <utility>
#include <vector>

namespace {
bool is_cave(ZoneId id) {
    return id.kind == ZoneId::Kind::CAVE_LEVEL;
}

std::pair<int, int> cell_of(float x, float y) {
    return {static_cast<int>(std::floor(x)), static_cast<int>(std::floor(y))};
}

std::pair<float, float> center_of(std::pair<int, int> cell) {
    return {static_cast<float>(cell.first) + 0.5f,
            static_cast<float>(cell.second) + 0.5f};
}

bool is_center(float coordinate) {
    return coordinate == std::floor(coordinate) + 0.5f;
}

void validate_accessibility(
        const Zone& zone,
        const std::vector<std::pair<float, float>>& arrivals) {
    for (const auto& arrival : arrivals) {
        if (!zone.is_walkable(arrival.first, arrival.second) ||
            zone.portal_at(arrival.first, arrival.second).has_value()) {
            throw std::invalid_argument("un punto de llegada no es valido");
        }

        const auto start = cell_of(arrival.first, arrival.second);
        std::queue<std::pair<int, int>> pending;
        std::set<std::pair<int, int>> visited;
        pending.push(start);
        visited.insert(start);

        while (!pending.empty()) {
            const auto [column, row] = pending.front();
            pending.pop();
            for (const auto& [dx, dy] :
                 std::vector<std::pair<int, int>>{
                     {1, 0}, {-1, 0}, {0, 1}, {0, -1}}) {
                const std::pair<int, int> next{column + dx, row + dy};
                if (visited.count(next) > 0) {
                    continue;
                }
                if (!zone.is_walkable(next.first + 0.5f, next.second + 0.5f)) {
                    continue;
                }
                visited.insert(next);
                bool is_portal = false;
                for (const auto& portal : zone.get_portals()) {
                    if (portal.cell == next) {
                        is_portal = true;
                        break;
                    }
                }
                if (!is_portal) {
                    pending.push(next);
                }
            }
        }

        for (const auto& portal : zone.get_portals()) {
            if (visited.count(portal.cell) == 0) {
                throw std::invalid_argument(
                    "un portal no es alcanzable desde un punto de llegada");
            }
        }
    }
}
}  // namespace

World::World(std::map<ZoneId, std::shared_ptr<const Zone>> zones_param):
        zones(std::move(zones_param)) {
    if (zone(ZoneId::FARM_ZONE) == nullptr) {
        throw std::invalid_argument("falta la zona de granja");
    }
    if (zone(ZoneId::VILLAGE_ZONE) == nullptr) {
        throw std::invalid_argument("falta la zona de pueblo");
    }

    for (const auto& [id, zone] : zones) {
        if (zone == nullptr) {
            throw std::invalid_argument("zona nula en el mundo");
        }
        if (zone->width() != ZONE_WIDTH || zone->height() != ZONE_HEIGHT) {
            throw std::invalid_argument("todas las zonas deben tener el mismo tamano");
        }
        if (zone->id() != id) {
            throw std::invalid_argument("la clave no coincide con el id de la zona");
        }
        if (is_cave(id)) {
            if (id.level < 1 || id.level > 10) {
                throw std::invalid_argument(
                    "nivel de caverna fuera de rango");
            }
        } else if (id.level != 0) {
            throw std::invalid_argument(
                "una zona que no es caverna debe tener nivel 0");
        }

        if (!is_cave(id)) {
            for (const Portal& portal : zone->get_portals()) {
                if (portal.type != PortalType::DUNGEON_DOOR &&
                    portal.type != PortalType::ROAD) {
                    throw std::invalid_argument(
                        "una zona de superficie solo puede tener puertas y caminos");
                }
            }
        } else {
            for (const Portal& portal : zone->get_portals()) {
                if (portal.type == PortalType::DUNGEON_DOOR ||
                    portal.type == PortalType::ROAD) {
                    throw std::invalid_argument(
                        "una zona de caverna no puede tener puertas ni caminos");
                }
            }
        }
    }

    const auto farm = zone(ZoneId::FARM_ZONE);
    const auto village = zone(ZoneId::VILLAGE_ZONE);
    int farm_roads = 0;
    int village_roads = 0;
    for (const Portal& portal : farm->get_portals()) {
        if (portal.type == PortalType::ROAD) {
            ++farm_roads;
            if (portal.destination != ZoneId::VILLAGE_ZONE) {
                throw std::invalid_argument(
                    "el camino de la granja debe llevar al pueblo");
            }
        }
    }
    for (const Portal& portal : village->get_portals()) {
        if (portal.type == PortalType::ROAD) {
            ++village_roads;
            if (portal.destination != ZoneId::FARM_ZONE) {
                throw std::invalid_argument(
                    "el camino del pueblo debe llevar a la granja");
            }
        }
    }
    if (farm_roads != 1 || village_roads != 1) {
        throw std::invalid_argument(
            "la granja y el pueblo deben tener exactamente un camino");
    }

    int entrance_count = 0;
    ZoneId surface_id = ZoneId::FARM_ZONE;
    std::pair<int, int> entrance_cell;
    std::pair<float, float> dungeon_arrival;
    for (const auto& entry : zones) {
        for (const Portal& portal : entry.second->get_portals()) {
            if (!is_center(portal.arrival.first) ||
                !is_center(portal.arrival.second)) {
                throw std::invalid_argument(
                    "la llegada de un portal debe ser centro de celda");
            }
            if (portal.type == PortalType::DUNGEON_DOOR) {
                if (is_cave(entry.first) ||
                    portal.destination !=
                        ZoneId{ZoneId::Kind::CAVE_LEVEL, 1}) {
                    throw std::invalid_argument(
                        "la puerta de entrada debe llevar al nivel 1");
                }
                ++entrance_count;
                surface_id = entry.first;
                entrance_cell = portal.cell;
                dungeon_arrival = portal.arrival;
            }
        }
    }
    if (entrance_count != 1) {
        throw std::invalid_argument(
            "el mundo debe tener exactamente una puerta de entrada");
    }

    const auto surface_zone = zone(surface_id);
    int exit_count = 0;
    std::optional<std::pair<int, int>> surface_arrival_cell;
    for (int level = 1; level <= 10; ++level) {
        const ZoneId level_id{ZoneId::Kind::CAVE_LEVEL, level};
        const auto current = zone(level_id);
        if (current == nullptr) {
            throw std::invalid_argument("faltan niveles de la mazmorra");
        }
        int stairs = 0;
        int stairs_up = 0;
        int stairs_down = 0;
        int exits = 0;
        for (const Portal& portal : current->get_portals()) {
            if (portal.type == PortalType::DUNGEON_EXIT) {
                ++exits;
                ++exit_count;
                if (portal.destination != surface_id) {
                    throw std::invalid_argument(
                        "una salida no vuelve a la zona de superficie");
                }
                const auto arrival_cell = cell_of(
                    portal.arrival.first, portal.arrival.second);
                if (!surface_arrival_cell.has_value()) {
                    surface_arrival_cell = arrival_cell;
                } else if (*surface_arrival_cell != arrival_cell) {
                    throw std::invalid_argument(
                        "las salidas no llegan a la misma celda de superficie");
                }
            } else if (portal.type == PortalType::STAIR) {
                ++stairs;
                if (portal.destination.kind != ZoneId::Kind::CAVE_LEVEL ||
                    std::abs(portal.destination.level - level) != 1) {
                    throw std::invalid_argument(
                        "una escalera debe conectar niveles consecutivos");
                }
                if (portal.destination.level == level + 1) {
                    ++stairs_down;
                } else {
                    ++stairs_up;
                }
            }
        }
        const int expected_stairs = (level == 1 || level == 10) ? 1 : 2;
        const int expected_up = level > 1 ? 1 : 0;
        const int expected_down = level < 10 ? 1 : 0;
        if (stairs != expected_stairs || stairs_up != expected_up ||
            stairs_down != expected_down || exits != 1) {
            throw std::invalid_argument(
                "cantidad invalida de portales en la mazmorra");
        }
    }
    if (exit_count != 10 || !surface_arrival_cell.has_value()) {
        throw std::invalid_argument(
            "la mazmorra debe tener una salida por nivel");
    }
    if (*surface_arrival_cell == entrance_cell) {
        throw std::invalid_argument(
            "la llegada de superficie no puede ser la puerta de entrada");
    }

    const auto surface_arrival = center_of(*surface_arrival_cell);
    if (!surface_zone->is_walkable(surface_arrival.first, surface_arrival.second) ||
        surface_zone->portal_at(surface_arrival.first, surface_arrival.second)) {
        throw std::invalid_argument(
            "la llegada de superficie debe estar en piso y no ser portal");
    }

    for (const auto& entry : zones) {
        for (const Portal& portal : entry.second->get_portals()) {
            const auto destination = this->zone(portal.destination);
            if (destination == nullptr ||
                !destination->is_walkable(portal.arrival.first,
                                          portal.arrival.second) ||
                destination->portal_at(portal.arrival.first,
                                       portal.arrival.second)) {
                throw std::invalid_argument("destino de portal invalido");
            }
        }
    }

    for (int level = 1; level < 10; ++level) {
        const auto lower = zone({ZoneId::Kind::CAVE_LEVEL, level});
        const auto upper = zone({ZoneId::Kind::CAVE_LEVEL, level + 1});
        for (const Portal& portal : lower->get_portals()) {
            if (portal.type != PortalType::STAIR ||
                portal.destination.level != level + 1) {
                continue;
            }
            bool reciprocal = false;
            for (const Portal& other : upper->get_portals()) {
                if (other.type == PortalType::STAIR &&
                    other.destination.level == level) {
                    reciprocal = true;
                }
            }
            if (!reciprocal) {
                throw std::invalid_argument("escalera sin reciprocidad");
            }
        }
    }

    std::map<ZoneId, std::vector<std::pair<float, float>>> arrivals;
    arrivals[surface_id].push_back(surface_arrival);
    arrivals[{ZoneId::Kind::CAVE_LEVEL, 1}].push_back(dungeon_arrival);
    for (const auto& [id, zone] : zones) {
        for (const Portal& portal : zone->get_portals()) {
            arrivals[portal.destination].push_back(portal.arrival);
        }
    }
    for (const auto& [id, zone] : zones) {
        validate_accessibility(*zone, arrivals[id]);
    }
}

std::shared_ptr<const Zone> World::zone(ZoneId id) const {
    const auto found = zones.find(id);
    if (found == zones.end()) {
        return nullptr;
    }
    return found->second;
}

bool World::is_walkable(ZoneId id, float x, float y) const {
    const auto selected_zone = zone(id);
    return selected_zone != nullptr && selected_zone->is_walkable(x, y);
}

std::optional<Portal> World::portal_at(ZoneId id, float x, float y) const {
    const auto selected_zone = zone(id);
    if (selected_zone == nullptr) {
        return std::nullopt;
    }
    return selected_zone->portal_at(x, y);
}

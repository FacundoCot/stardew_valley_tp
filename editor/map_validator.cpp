#include "map_validator.h"

#include <string>

#include <QGraphicsItem>

#include "map_definitions.h"
#include "map_file.h"
#include "map_rules.h"

std::string MapValidator::zone_name(int zone) const {
    const auto found = ZONE_NAMES.find(zone);
    if (found != ZONE_NAMES.end()) {
        return found->second;
    }
    return "Caverna " + std::to_string(zone - ZONE_FIRST_CAVE + 1);
}

bool MapValidator::needs(int zone, const std::string& element_id) const {
    return REQUIRED_ELEMENTS.at(zone).contains(element_id);
}

int MapValidator::type_of(const std::string& element_id) const {
    for (int type = 0; type < ELEMENT_COUNT; ++type) {
        if (ELEMENT_TYPES[type].id == element_id) {
            return type;
        }
    }
    return -1;
}

std::vector<int> MapValidator::count_elements(const MapScene* scene) const {
    std::vector<int> counts(ELEMENT_COUNT, 0);

    for (int row = 0; row < scene->row_count(); ++row) {
        for (int column = 0; column < scene->column_count(); ++column) {
            const QGraphicsItem* item = scene->element_item(column, row);
            if (item != nullptr) {
                counts[static_cast<size_t>(item->data(ELEMENT_DATA_TYPE).toInt())]++;
            }
        }
    }
    return counts;
}

void MapValidator::check_solid(int zone, const MapScene* scene,
                               std::vector<std::string>& problems) const {
    for (int row = 0; row < scene->row_count(); ++row) {
        for (int column = 0; column < scene->column_count(); ++column) {
            const QGraphicsItem* item = scene->element_item(column, row);
            if (item == nullptr) {
                continue;
            }

            const ElementType& type = ELEMENT_TYPES[item->data(ELEMENT_DATA_TYPE).toInt()];
            if (!FLOOR_ELEMENTS.contains(type.id)) {
                continue;
            }

            for (int layer = 0; layer < LAYER_COUNT; ++layer) {
                if (LAYER_SOLID[layer] && scene->tile_item(layer, column, row) != nullptr) {
                    problems.push_back(zone_name(zone) + ": " + type.name + " en (" + std::to_string(column) + 
                                        ", " + std::to_string(row) + ") está sobre un tile sólido");
                    break;
                }
            }
        }
    }
}

void MapValidator::check_cave_elements(int zone, const std::vector<int>& counts,
                                       std::vector<std::string>& problems) const {
    bool is_cave = (zone >= ZONE_FIRST_CAVE && zone < ZONE_HOUSE);
    const int level = zone - ZONE_FIRST_CAVE + 1;

    for (int type = 0; type < ELEMENT_COUNT; ++type) {
        const std::string& id = ELEMENT_TYPES[type].id;
        if (!CAVE_ONLY_ELEMENTS.contains(id) || counts[static_cast<size_t>(type)] == 0) {
            continue;
        }

        if (!is_cave) {
            problems.push_back(zone_name(zone) + ": " + ELEMENT_TYPES[type].name + " solo corresponde en las cavernas");
        } else if (id == "ore_gold" && level < GOLD_FIRST_LEVEL) {
            problems.push_back(zone_name(zone) + ": " + ELEMENT_TYPES[type].name + " solo puede aparecer desde la caverna " 
                                + std::to_string(GOLD_FIRST_LEVEL));
        }
    }
}

void MapValidator::exactly_one(int zone, const std::vector<int>& counts, const std::string& element_id,
                               std::vector<std::string>& problems) const {
    const int type = type_of(element_id);
    const int amount = counts[static_cast<size_t>(type)];
    if (amount == 0) {
        problems.push_back(zone_name(zone) + ": falta " + ELEMENT_TYPES[type].name);
    } else if (amount > 1) {
        problems.push_back(zone_name(zone) + ": hay mas de un " + ELEMENT_TYPES[type].name);
    }
}

void MapValidator::at_least_one(int zone, const std::vector<int>& counts, const std::string& element_id,
                                std::vector<std::string>& problems) const {
    const int type = type_of(element_id);
    if (counts[static_cast<size_t>(type)] == 0) {
        problems.push_back(zone_name(zone) + ": falta al menos un " + ELEMENT_TYPES[type].name);
    }
}

void MapValidator::at_least_one_of(int zone, const std::vector<int>& counts,
                                   const std::unordered_set<std::string>& element_ids,
                                   const std::string& what, std::vector<std::string>& problems) const {
    int total = 0;
    for (const std::string& id : element_ids) {
        total += counts[static_cast<size_t>(type_of(id))];
    }
    if (total == 0) {
        problems.push_back(zone_name(zone) + ": falta al menos un " + what);
    }
}

void MapValidator::none(int zone, const std::vector<int>& counts, const std::string& element_id,
                        std::vector<std::string>& problems) const {
    const int type = type_of(element_id);
    if (counts[static_cast<size_t>(type)] > 0) {
        problems.push_back(zone_name(zone) + ": sobra " + ELEMENT_TYPES[type].name);
    }
}

void MapValidator::check_zone(int zone, const MapScene* scene,
                              std::vector<std::string>& problems) const {
    const std::vector<int> counts = count_elements(scene);

    for (int type = 0; type < ELEMENT_COUNT; ++type) {
        const std::string& id = ELEMENT_TYPES[type].id;
        if (CAVE_ONLY_ELEMENTS.contains(id)) {
            continue; // reviso aparte al no ser únicos
        }

        if (needs(zone, id)) {
            exactly_one(zone, counts, id, problems);
        } else {
            none(zone, counts, id, problems);
        }
    }

    // al menos un spawn de enemigos (de cualquier tipo) por caverna
    bool is_cave = (zone >= ZONE_FIRST_CAVE && zone < ZONE_HOUSE);
    if (is_cave) {
        at_least_one_of(zone, counts, ENEMY_SPAWNS, "spawn de enemigo", problems);
    }

    check_cave_elements(zone, counts, problems);
    check_solid(zone, scene, problems);
}

std::vector<std::string> MapValidator::validate(const std::vector<MapScene*>& scenes) const {
    std::vector<std::string> problems;
    for (int zone = 0; zone < ZONE_COUNT; ++zone) {
        check_zone(zone, scenes[static_cast<size_t>(zone)], problems);
    }
    return problems;
}

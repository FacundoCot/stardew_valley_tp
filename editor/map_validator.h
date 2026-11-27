#ifndef MAP_VALIDATOR_H
#define MAP_VALIDATOR_H

#include <string>
#include <vector>

#include "map_scene.h"

class MapValidator {
private:
    std::string zone_name(int zone) const;

    bool needs(int zone, const std::string& element_id) const;

    int type_of(const std::string& element_id) const;

    std::vector<int> count_elements(const MapScene* scene) const;

    void check_solid(int zone, const MapScene* scene, std::vector<std::string>& problems) const;

    void check_cave_elements(int zone, const std::vector<int>& counts,
                             std::vector<std::string>& problems) const;

    void exactly_one(int zone, const std::vector<int>& counts, const std::string& element_id,
                     std::vector<std::string>& problems) const;
    void at_least_one(int zone, const std::vector<int>& counts, const std::string& element_id,
                      std::vector<std::string>& problems) const;
    void none(int zone, const std::vector<int>& counts, const std::string& element_id,
              std::vector<std::string>& problems) const;

    void check_zone(int zone, const MapScene* scene, std::vector<std::string>& problems) const;

public:
    std::vector<std::string> validate(const std::vector<MapScene*>& scenes) const;
};

#endif

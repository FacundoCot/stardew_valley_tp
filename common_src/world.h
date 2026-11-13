#ifndef WORLD_H
#define WORLD_H

#include <map>
#include <memory>
#include <optional>

#include "zone.h"

class World {
private:
    std::map<ZoneId, std::shared_ptr<const Zone>> zones;

public:
    explicit World(std::map<ZoneId, std::shared_ptr<const Zone>> zones);

    std::shared_ptr<const Zone> zone(ZoneId id) const;
    bool is_walkable(ZoneId id, float x, float y) const;
    std::optional<Portal> portal_at(ZoneId id, float x, float y) const;
};

#endif

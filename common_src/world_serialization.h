#ifndef WORLD_SERIALIZATION_H
#define WORLD_SERIALIZATION_H

#include <cstdint>
#include <utility>
#include <vector>

#include "world.h"

struct SerializedPortal {
    PortalType type;
    std::pair<int, int> cell;
    ZoneId destination;
    std::pair<float, float> arrival;
};

struct SerializedZone {
    ZoneId id;
    uint16_t width;
    uint16_t height;
    std::vector<Tile> tiles;
    std::vector<SerializedPortal> portals;
    Rectangle cabin{};
    std::vector<std::pair<float, float>> beds;
    std::vector<ZoneId::ZoneArea> areas;
};

struct SerializedWorld {
    std::vector<SerializedZone> zones;
};

std::vector<uint8_t> encode_world(const World& world);
SerializedWorld decode_world(const std::vector<uint8_t>& data);

#endif

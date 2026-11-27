#ifndef WORLD_SERIALIZATION_H
#define WORLD_SERIALIZATION_H

#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "world.h"
#include "merchant_pos.h"

struct SerializedPortal {
    PortalType type;
    std::pair<int, int> cell;
    ZoneId destination;
    std::pair<float, float> arrival;
};

struct SerializedVisualTile {
    int x;
    int y;
    std::string layer;
    std::string sheet;
    int col;
    int row;
};

struct SerializedStaticElement {
    StaticElementType type;
    std::pair<int, int> cell;
};

struct SerializedZone {
    ZoneId id;
    uint16_t width;
    uint16_t height;
    std::vector<Tile> tiles;
    std::vector<SerializedPortal> portals;
    Rectangle cabin{};
    std::optional<Rectangle> community_center;
    std::vector<std::pair<float, float>> beds;
    std::vector<ZoneId::ZoneArea> areas;
    std::vector<MerchantPosition> merchants;
    std::vector<SerializedVisualTile> visual_tiles;
    std::vector<SerializedStaticElement> static_elements;
};

struct SerializedWorld {
    std::vector<SerializedZone> zones;
};

std::vector<uint8_t> encode_world(const World& world);
SerializedWorld decode_world(const std::vector<uint8_t>& data);

#endif

#include "world_serialization.h"

#include <arpa/inet.h>

#include <cmath>
#include <cstddef>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <string>

#include "cave_level.h"
#include "farm_zone.h"
#include "village_zone.h"

namespace {
void append_u8(std::vector<uint8_t>& data, uint8_t value) {
    data.push_back(value);
}

void append_u16(std::vector<uint8_t>& data, uint16_t value) {
    const uint16_t network_value = htons(value);
    const auto* bytes = reinterpret_cast<const uint8_t*>(&network_value);
    data.insert(data.end(), bytes, bytes + sizeof(network_value));
}

void append_i32(std::vector<uint8_t>& data, int32_t value) {
    const uint32_t network_value = htonl(static_cast<uint32_t>(value));
    const auto* bytes = reinterpret_cast<const uint8_t*>(&network_value);
    data.insert(data.end(), bytes, bytes + sizeof(network_value));
}

int32_t coordinate_to_i32(float value) {
    if (!std::isfinite(value) ||
        value * 100.0f < std::numeric_limits<int32_t>::min() ||
        value * 100.0f > std::numeric_limits<int32_t>::max()) {
        throw std::runtime_error("coordenada fuera de rango");
    }
    return static_cast<int32_t>(std::lround(value * 100.0f));
}

uint8_t checked_count(size_t count, const char* description) {
    if (count > std::numeric_limits<uint8_t>::max()) {
        throw std::runtime_error(std::string(description) + " demasiado grande");
    }
    return static_cast<uint8_t>(count);
}

const std::vector<Tile>& zone_tiles(const Zone& zone) {
    if (const auto* farm = dynamic_cast<const FarmZone*>(&zone)) {
        return farm->grid();
    }
    if (const auto* village = dynamic_cast<const VillageZone*>(&zone)) {
        return village->grid();
    }
    if (const auto* cave = dynamic_cast<const CaveLevel*>(&zone)) {
        return cave->grid();
    }
    throw std::runtime_error("tipo de zona desconocido");
}

void append_portal(std::vector<uint8_t>& data, const Portal& portal) {
    append_u8(data, static_cast<uint8_t>(portal.type));
    append_i32(data, portal.cell.first);
    append_i32(data, portal.cell.second);
    append_u8(data, static_cast<uint8_t>(portal.destination.kind));
    append_u8(data, static_cast<uint8_t>(portal.destination.level));
    append_i32(data, coordinate_to_i32(portal.arrival.first));
    append_i32(data, coordinate_to_i32(portal.arrival.second));
}

class Reader {
private:
    const std::vector<uint8_t>& data;
    size_t position = 0;

    void require(size_t size) {
        if (size > data.size() - position) {
            throw std::runtime_error("WORLD_INFO truncado");
        }
    }

public:
    explicit Reader(const std::vector<uint8_t>& data_param): data(data_param) {}

    uint8_t u8() {
        require(1);
        return data[position++];
    }

    uint16_t u16() {
        require(2);
        uint16_t value;
        std::memcpy(&value, data.data() + position, sizeof(value));
        position += sizeof(value);
        return ntohs(value);
    }

    int32_t i32() {
        require(4);
        uint32_t value;
        std::memcpy(&value, data.data() + position, sizeof(value));
        position += sizeof(value);
        return static_cast<int32_t>(ntohl(value));
    }

    bool done() const {
        return position == data.size();
    }
};

ZoneId::Kind decode_kind(uint8_t value) {
    if (value > static_cast<uint8_t>(ZoneId::Kind::CAVE_LEVEL)) {
        throw std::runtime_error("kind de zona invalido");
    }
    return static_cast<ZoneId::Kind>(value);
}

Tile decode_tile(uint8_t value) {
    if (value > static_cast<uint8_t>(Tile::FLOOR)) {
        throw std::runtime_error("tile invalido");
    }
    return static_cast<Tile>(value);
}

PortalType decode_portal_type(uint8_t value) {
    if (value > static_cast<uint8_t>(PortalType::ROAD)) {
        throw std::runtime_error("tipo de portal invalido");
    }
    return static_cast<PortalType>(value);
}
}  // namespace

std::vector<uint8_t> encode_world(const World& world) {
    const auto& zones = world.zones_data();
    std::vector<uint8_t> data;
    append_u8(data, 7);
    append_u8(data, checked_count(zones.size(), "cantidad de zonas"));

    for (const auto& [id, zone] : zones) {
        append_u8(data, static_cast<uint8_t>(id.kind));
        append_u8(data, static_cast<uint8_t>(id.level));
        append_u16(data, static_cast<uint16_t>(zone->width()));
        append_u16(data, static_cast<uint16_t>(zone->height()));

        const auto& tiles = zone_tiles(*zone);
        if (tiles.size() != static_cast<size_t>(zone->width()) *
                                 static_cast<size_t>(zone->height())) {
            throw std::runtime_error("grilla de zona invalida");
        }
        for (Tile tile : tiles) {
            append_u8(data, static_cast<uint8_t>(tile));
        }

        append_u8(data, checked_count(zone->get_portals().size(),
                                      "cantidad de portales"));
        for (const Portal& portal : zone->get_portals()) {
            append_portal(data, portal);
        }

        if (const auto* farm = dynamic_cast<const FarmZone*>(zone.get())) {
            const Rectangle cabin = farm->cabin();
            append_i32(data, cabin.x);
            append_i32(data, cabin.y);
            append_i32(data, cabin.width);
            append_i32(data, cabin.height);
            append_u8(data, checked_count(farm->beds().size(),
                                          "cantidad de camas"));
            for (const auto& bed : farm->beds()) {
                append_i32(data, coordinate_to_i32(bed.first));
                append_i32(data, coordinate_to_i32(bed.second));
            }
        } else if (const auto* village =
                       dynamic_cast<const VillageZone*>(zone.get())) {
            append_u8(data, checked_count(village->areas().size(),
                                          "cantidad de areas"));
            for (const auto& area : village->areas()) {
                append_u8(data, static_cast<uint8_t>(area.type));
                append_i32(data, area.x);
                append_i32(data, area.y);
                append_i32(data, area.width);
                append_i32(data, area.height);
            }
        }
    }
    return data;
}

SerializedWorld decode_world(const std::vector<uint8_t>& data) {
    Reader reader(data);
    if (reader.u8() != 7) {
        throw std::runtime_error("tipo de WORLD_INFO invalido");
    }

    SerializedWorld world;
    const uint8_t zone_count = reader.u8();
    for (uint8_t index = 0; index < zone_count; ++index) {
        SerializedZone zone;
        zone.id.kind = decode_kind(reader.u8());
        zone.id.level = reader.u8();
        zone.width = reader.u16();
        zone.height = reader.u16();
        if (zone.width == 0 || zone.height == 0) {
            throw std::runtime_error("tamano de zona invalido");
        }
        const size_t tile_count = static_cast<size_t>(zone.width) * zone.height;
        zone.tiles.reserve(tile_count);
        for (size_t tile = 0; tile < tile_count; ++tile) {
            zone.tiles.push_back(decode_tile(reader.u8()));
        }

        const uint8_t portal_count = reader.u8();
        zone.portals.reserve(portal_count);
        for (uint8_t portal_index = 0; portal_index < portal_count;
             ++portal_index) {
            SerializedPortal portal;
            portal.type = decode_portal_type(reader.u8());
            portal.cell = {reader.i32(), reader.i32()};
            portal.destination.kind = decode_kind(reader.u8());
            portal.destination.level = reader.u8();
            portal.arrival = {reader.i32() / 100.0f, reader.i32() / 100.0f};
            zone.portals.push_back(portal);
        }

        if (zone.id.kind == ZoneId::Kind::FARM) {
            zone.cabin = {reader.i32(), reader.i32(), reader.i32(), reader.i32()};
            const uint8_t bed_count = reader.u8();
            zone.beds.reserve(bed_count);
            for (uint8_t bed = 0; bed < bed_count; ++bed) {
                zone.beds.emplace_back(reader.i32() / 100.0f,
                                       reader.i32() / 100.0f);
            }
        } else if (zone.id.kind == ZoneId::Kind::VILLAGE) {
            const uint8_t area_count = reader.u8();
            zone.areas.reserve(area_count);
            for (uint8_t area = 0; area < area_count; ++area) {
                ZoneId::ZoneArea value;
                const uint8_t type = reader.u8();
                if (type > static_cast<uint8_t>(
                               ZoneId::AreaType::COMMUNITY_CENTER)) {
                    throw std::runtime_error("tipo de area invalido");
                }
                value.type = static_cast<ZoneId::AreaType>(type);
                value.x = reader.i32();
                value.y = reader.i32();
                value.width = reader.i32();
                value.height = reader.i32();
                zone.areas.push_back(value);
            }
        }
        world.zones.push_back(std::move(zone));
    }
    if (!reader.done()) {
        throw std::runtime_error("datos extra al final de WORLD_INFO");
    }
    return world;
}

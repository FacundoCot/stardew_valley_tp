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
#include "interior_zone.h"
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

void append_string(std::vector<uint8_t>& data, const std::string& value) {
    if (value.size() > std::numeric_limits<uint16_t>::max()) {
        throw std::runtime_error("cadena demasiado grande");
    }
    append_u16(data, static_cast<uint16_t>(value.size()));
    data.insert(data.end(), value.begin(), value.end());
}

int32_t coordinate_to_i32(float value) {
    if (!std::isfinite(value) ||
        value * 100.0f < std::numeric_limits<int32_t>::min() ||
        value * 100.0f > std::numeric_limits<int32_t>::max()) {
        throw std::runtime_error("coordenada fuera de rango");
    }
    return static_cast<int32_t>(std::lround(value * 100.0f));
}

uint16_t checked_count_u16(size_t count, const char* description) {
    if (count > std::numeric_limits<uint16_t>::max()) {
        throw std::runtime_error(std::string(description) + " demasiado grande");
    }
    return static_cast<uint16_t>(count);
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
    if (const auto* interior = dynamic_cast<const InteriorZone*>(&zone)) {
        return interior->grid();
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

    std::string string() {
        const uint16_t length = u16();
        require(length);
        std::string value(reinterpret_cast<const char*>(data.data() + position), length);
        position += length;
        return value;
    }

    bool done() const {
        return position == data.size();
    }
};

ZoneId::Kind decode_kind(uint8_t value) {
    switch (value) {
        case static_cast<uint8_t>(ZoneId::Kind::FARM):
        case static_cast<uint8_t>(ZoneId::Kind::VILLAGE):
        case static_cast<uint8_t>(ZoneId::Kind::CAVE_LEVEL):
        case static_cast<uint8_t>(ZoneId::Kind::HOUSE):
        case static_cast<uint8_t>(ZoneId::Kind::COMMUNITY_CENTER):
            return static_cast<ZoneId::Kind>(value);
        default:
            throw std::runtime_error("kind de zona invalido");
    }
}

Tile decode_tile(uint8_t value) {
    if (value > static_cast<uint8_t>(Tile::TREE)) {
        throw std::runtime_error("tile invalido");
    }
    return static_cast<Tile>(value);
}

PortalType decode_portal_type(uint8_t value) {
    switch (value) {
        case static_cast<uint8_t>(PortalType::STAIR):
        case static_cast<uint8_t>(PortalType::DUNGEON_DOOR):
        case static_cast<uint8_t>(PortalType::DUNGEON_EXIT):
        case static_cast<uint8_t>(PortalType::ROAD):
        case static_cast<uint8_t>(PortalType::HOUSE_DOOR):
        case static_cast<uint8_t>(PortalType::HOUSE_EXIT):
        case static_cast<uint8_t>(PortalType::CENTER_DOOR):
        case static_cast<uint8_t>(PortalType::CENTER_EXIT):
            return static_cast<PortalType>(value);
        default:
            throw std::runtime_error("tipo de portal invalido");
    }
}

StaticElementType decode_static_element_type(uint8_t value) {
    if (value <= static_cast<uint8_t>(StaticElementType::TRAP_FLAME_DOWN)) {
        return static_cast<StaticElementType>(value);
    }
    throw std::runtime_error("tipo de elemento estatico invalido");
}
}  // namespace

std::vector<uint8_t> encode_world(const World& world) {
    const auto& zones = world.zones_data();
    std::vector<uint8_t> data;
    append_u8(data, 11);
    append_u8(data, checked_count(zones.size(), "cantidad de zonas"));

    for (const auto& [id, zone] : zones) {
        if (zone->width() <= 0 || zone->height() <= 0 ||
            zone->width() > std::numeric_limits<uint16_t>::max() ||
            zone->height() > std::numeric_limits<uint16_t>::max()) {
            throw std::runtime_error("tamano de zona fuera de rango");
        }
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

        append_u16(data, checked_count_u16(zone->get_visual_tiles().size(),
                                           "cantidad de tiles visuales"));
        for (const VisualTile& visual : zone->get_visual_tiles()) {
            append_i32(data, visual.x);
            append_i32(data, visual.y);
            append_string(data, visual.layer);
            append_string(data, visual.sheet);
            append_i32(data, visual.col);
            append_i32(data, visual.row);
        }

        append_u8(data, checked_count(zone->get_portals().size(),
                                      "cantidad de portales"));
        for (const Portal& portal : zone->get_portals()) {
            append_portal(data, portal);
        }

        append_u16(data, checked_count_u16(zone->get_static_elements().size(),
                                           "cantidad de elementos estaticos"));
        for (const StaticElement& element : zone->get_static_elements()) {
            append_u8(data, static_cast<uint8_t>(element.type));
            append_i32(data, element.cell.first);
            append_i32(data, element.cell.second);
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
            append_u8(data, checked_count(village->merchants().size(), "cantidad de comerciantes"));

            for (const MerchantPosition& merchant : village->merchants()) {
                append_u8(data, static_cast<uint8_t>(merchant.type));
                append_i32(data, coordinate_to_i32(merchant.x));
                append_i32(data, coordinate_to_i32(merchant.y));
            }
            const auto community_center = village->community_center();
            append_u8(data, community_center.has_value() ? 1 : 0);
            if (community_center.has_value()) {
                append_i32(data, community_center->x);
                append_i32(data, community_center->y);
                append_i32(data, community_center->width);
                append_i32(data, community_center->height);
            }
        }
    }
    return data;
}

SerializedWorld decode_world(const std::vector<uint8_t>& data) {
    Reader reader(data);
    if (reader.u8() != 11) {
        throw std::runtime_error("version de WORLD_INFO invalida");
    }

    SerializedWorld world;
    const uint8_t zone_count = reader.u8();
    for (uint8_t index = 0; index < zone_count; ++index) {
        SerializedZone zone;
        zone.id.kind = decode_kind(reader.u8());
        zone.id.level = reader.u8();
        if (zone.id.kind == ZoneId::Kind::CAVE_LEVEL) {
            if (zone.id.level < 1 || zone.id.level > 10) {
                throw std::runtime_error("nivel de caverna invalido");
            }
        } else if (zone.id.level != 0) {
            throw std::runtime_error("nivel de zona invalido");
        }
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

        const uint16_t visual_count = reader.u16();
        zone.visual_tiles.reserve(visual_count);
        for (uint16_t visual_index = 0; visual_index < visual_count;
             ++visual_index) {
            SerializedVisualTile visual{
                reader.i32(), reader.i32(), reader.string(), reader.string(),
                reader.i32(), reader.i32()};
            zone.visual_tiles.push_back(std::move(visual));
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
            const int32_t arrival_x = reader.i32();
            const int32_t arrival_y = reader.i32();
            portal.arrival = {arrival_x / 100.0f, arrival_y / 100.0f};
            zone.portals.push_back(portal);
        }

        const uint16_t static_count = reader.u16();
        zone.static_elements.reserve(static_count);
        for (uint16_t static_index = 0; static_index < static_count;
             ++static_index) {
            SerializedStaticElement element;
            element.type = decode_static_element_type(reader.u8());
            element.cell = {reader.i32(), reader.i32()};
            zone.static_elements.push_back(element);
        }

        if (zone.id.kind == ZoneId::Kind::FARM) {
            zone.cabin = {reader.i32(), reader.i32(), reader.i32(), reader.i32()};
            const uint8_t bed_count = reader.u8();
            zone.beds.reserve(bed_count);
            for (uint8_t bed = 0; bed < bed_count; ++bed) {
                const int32_t bed_x = reader.i32();
                const int32_t bed_y = reader.i32();
                zone.beds.emplace_back(bed_x / 100.0f, bed_y / 100.0f);
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
            // Comerciantes
            const uint8_t merchant_count = reader.u8();
            zone.merchants.reserve(merchant_count);

            for (uint8_t i = 0; i < merchant_count; ++i) {
                const uint8_t type = reader.u8();

                if (type > static_cast<uint8_t>(MerchantType::CLINT)) {
                    throw std::runtime_error("Comerciante invalido");
                }

                const int32_t x = reader.i32();
                const int32_t y = reader.i32();

                zone.merchants.push_back(MerchantPosition{
                    static_cast<MerchantType>(type),
                    x / 100.0f,
                    y / 100.0f
                });
            }

            const uint8_t has_community_center = reader.u8();
            if (has_community_center > 1) {
                throw std::runtime_error(
                    "presencia del centro comunitario invalida");
            }
            if (has_community_center == 1) {
                Rectangle center{
                    reader.i32(), reader.i32(), reader.i32(), reader.i32()};
                if (center.width < 1 || center.height < 1 || center.x < 0 ||
                    center.y < 0 || center.width > zone.width ||
                    center.height > zone.height ||
                    center.x > static_cast<int>(zone.width) - center.width ||
                    center.y > static_cast<int>(zone.height) - center.height) {
                    throw std::runtime_error(
                        "rectangulo del centro comunitario invalido");
                }
                zone.community_center = center;
            }
        }
        world.zones.push_back(std::move(zone));
    }
    if (!reader.done()) {
        throw std::runtime_error("datos extra al final de WORLD_INFO");
    }
    return world;
}

#include "map_visual.h"
#include "visual.h"
#include "../common_src/world_serialization.h"
#include "loader.h"
#include "camera.h"
#include "map_constants.h"

#include <SDL2pp/SDL2pp.hh>

MapVisual::MapVisual(Visual& visual, Loader& loader, SerializedWorld&& world) : visual(visual), loader(loader), world(std::move(world)) {}


void MapVisual::draw_map(Camera& camera) {
    for(SerializedZone &zone : world.zones) {
        switch (zone.id.kind) {
            case ZoneId::Kind::FARM:
                draw_farm(zone, camera);
            case ZoneId::Kind::VILLAGE:
            case ZoneId::Kind::CAVE_LEVEL:
                continue;
        }
    }
    return;
}

void MapVisual::draw_farm(SerializedZone& zone, Camera& camera) {
    SDL2pp::Texture& ground = loader.get_texture("ground");
    //SDL2pp::Renderer& renderer = visual.get_renderer();

    for(uint16_t row = 0; row < zone.height ; row ++) {
        for (uint16_t col = 0; col < zone.width; col++) {
            Tile tile = zone.tiles[row*zone.width + col];

            int dest_x = col * TILE_SIZE - camera.get_rect().x;
            int dest_y = row * TILE_SIZE - camera.get_rect().y;
            int dest_w = TILE_SIZE;
            int dest_h = TILE_SIZE;

            
            int src_x = 7, src_y = 136, src_w = 34, src_h = 34;
            if (tile == Tile::FLOOR) {
                src_x = 7;
                src_y = 136;

            } else {
                src_x = 0;
                src_y = 0;
            }

            visual.draw(ground, src_x, src_y, src_h, src_w, dest_x, dest_y, dest_h, dest_w);

        }
    }


}
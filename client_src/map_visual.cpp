#include "map_visual.h"
#include "visual.h"
#include "../common_src/world_serialization.h"
#include "loader.h"
#include "camera.h"
#include "../common_src/zone.h"
#include "map_constants.h"

#include <SDL2pp/SDL2pp.hh>

MapVisual::MapVisual(Visual& visual, Loader& loader, SerializedWorld&& world) : visual(visual), 
                    loader(loader), world(std::move(world)) {}


void MapVisual::draw_map(ZoneId& zone, Camera& camera) {
    SerializedZone zone_ser = get_zone(zone);
    switch (zone.kind) {
        case ZoneId::Kind::FARM:
            draw_farm(zone_ser, camera);
            break;
         case ZoneId::Kind::VILLAGE:
            draw_village(zone_ser, camera);
            break;
        case ZoneId::Kind::CAVE_LEVEL:
            return;
    }
    
    return;
}

void MapVisual::draw_farm(SerializedZone& zone, Camera& camera) {
    SDL2pp::Texture& ground = loader.get_texture("ground");

    for(uint16_t row = 0; row < zone.height ; row ++) {
        for (uint16_t col = 0; col < zone.width; col++) {
            Tile tile = zone.tiles[row*zone.width + col];

            int dest_x = col * TILE_SIZE - camera.get_rect().x;
            int dest_y = row * TILE_SIZE - camera.get_rect().y;
            int dest_w = TILE_SIZE;
            int dest_h = TILE_SIZE;

            
            int src_x, src_y, src_w = 16, src_h = 16;
            if (tile == Tile::FLOOR || tile == Tile::TREE || tile == Tile::PATH || tile == Tile::GRASS) {
                src_x = 14;
                src_y = 62;
                visual.draw(ground, src_x, src_y, src_h, src_w, dest_x, dest_y, dest_h, dest_w);
            }  
        }
    }
    draw_grass(zone, camera);
    draw_tree(zone, camera);
    draw_path(zone, camera); 
    draw_cabin(zone, camera);
    draw_walls(zone, camera);
    draw_portals(zone, camera);

}

void MapVisual::draw_tree(SerializedZone& zone, Camera& camera) {
    SDL2pp::Texture& tree = loader.get_texture("tree1");

    for(uint16_t row = 0; row < zone.height ; row ++) {
        for (uint16_t col = 0; col < zone.width; col++) {
            Tile tile = zone.tiles[row*zone.width + col];

            int dest_x = col * TILE_SIZE - camera.get_rect().x;
            int dest_y = row * TILE_SIZE - camera.get_rect().y;
            int dest_w = W_TREE*3;
            int dest_h = H_TREE*3;

            if (tile == Tile::TREE) {
                int src_x = 0;
                int src_y = 0;
                int src_w = W_TREE;
                int src_h = H_TREE;
                visual.draw(tree, src_x, src_y, src_h, src_w, dest_x, dest_y, dest_h, dest_w);

            }  
        }
    }

}


void MapVisual::draw_path(SerializedZone& zone, Camera& camera) {
    SDL2pp::Texture& path = loader.get_texture("ground");

    for(uint16_t row = 0; row < zone.height ; row ++) {
        for (uint16_t col = 0; col < zone.width; col++) {
            Tile tile = zone.tiles[row*zone.width + col];

            int dest_x = col * TILE_SIZE - camera.get_rect().x;
            int dest_y = row * TILE_SIZE - camera.get_rect().y;
            int dest_w = TILE_SIZE;
            int dest_h = TILE_SIZE;

            if (tile == Tile::PATH) {
                int src_x = 186;
                int src_y = 97;
                int src_w = W_PATH;
                int src_h = H_PATH;
                visual.draw(path, src_x, src_y, src_h, src_w, dest_x, dest_y, dest_h, dest_w);

            }  
        }
    } 
}

void MapVisual::draw_cabin(SerializedZone& zone, Camera& camera) {
    SDL2pp::Texture& cabin = loader.get_texture("cabin");

    int dest_x = zone.cabin.x*TILE_SIZE - camera.get_rect().x;
    int dest_y = zone.cabin.y*TILE_SIZE - camera.get_rect().y;

    int dest_w = zone.cabin.width*TILE_SIZE;
    int dest_h = zone.cabin.height*TILE_SIZE;

    visual.draw(cabin, X_CABIN, Y_CABIN, H_CABIN, W_CABIN, dest_x, dest_y, dest_w, dest_h);

}

void MapVisual::draw_grass(SerializedZone& zone, Camera& camera) {
    SDL2pp::Texture& grass = loader.get_texture("grass");

    int src_x = 31;
    int src_y = 2;
    int src_w = 16;
    int src_h = 16;

    int dest_w = TILE_SIZE;
    int dest_h = TILE_SIZE;

    for(uint16_t row = 0; row < zone.height ; row ++) {
        for (uint16_t col = 0; col < zone.width; col++) {
            Tile tile = zone.tiles[row*zone.width + col];

            int dest_x = col * TILE_SIZE - camera.get_rect().x;
            int dest_y = row * TILE_SIZE - camera.get_rect().y;


            if (tile == Tile::GRASS) {
                for (uint8_t i = 0; i < 6; i++) {
                    dest_x += GRASS_OFFSET[i][0];
                    dest_y += GRASS_OFFSET[i][1];
                    visual.draw(grass, src_x, src_y, src_h, src_w, dest_x, dest_y, dest_h, dest_w);
                }
            }  
        }
    }    
}

void MapVisual::draw_walls(SerializedZone& zone, Camera& camera) {
    SDL2pp::Texture& wall = loader.get_texture("rock");

    int src_x = 0;
    int src_y = 0;
    int src_w = 10;
    int src_h = 10;

    int dest_w = src_w*2;
    int dest_h = src_h*2;

    for(uint16_t row = 0; row < zone.height ; row ++) {
        for (uint16_t col = 0; col < zone.width; col++) {
            Tile tile = zone.tiles[row*zone.width + col];

            int dest_x = col * TILE_SIZE - camera.get_rect().x;
            int dest_y = row * TILE_SIZE - camera.get_rect().y;

            if (tile == Tile::WALL) {
                visual.draw(wall, src_x, src_y, src_h, src_w, dest_x, dest_y, dest_h, dest_w);
            }
            
        }
    }  
}


void MapVisual::draw_village(SerializedZone& zone, Camera& camera) {
    SDL2pp::Texture& ground = loader.get_texture("ground");

    for(uint16_t row = 0; row < zone.height ; row ++) {
        for (uint16_t col = 0; col < zone.width; col++) {
            Tile tile = zone.tiles[row*zone.width + col];

            int dest_x = col * TILE_SIZE - camera.get_rect().x;
            int dest_y = row * TILE_SIZE - camera.get_rect().y;
            int dest_w = TILE_SIZE;
            int dest_h = TILE_SIZE;

            
            int src_x, src_y, src_w = 16, src_h = 16;
            if (tile == Tile::FLOOR || tile == Tile::TREE || tile == Tile::PATH || tile == Tile::GRASS) {
                src_x = 14;
                src_y = 62;
                visual.draw(ground, src_x, src_y, src_h, src_w, dest_x, dest_y, dest_h, dest_w);
            }  
        }
    }
    draw_grass(zone, camera);
    draw_tree(zone, camera);
    draw_path(zone, camera);
}

SerializedZone MapVisual::get_zone(ZoneId& kind) {
    for (const auto& z : world.zones) {
        if (z.id.kind == kind.kind) {
            return z;
        }
    }
    throw std::runtime_error("Error at MapVisual. There is no coincidence with id received");

}

void MapVisual::draw_portals(SerializedZone& zone, Camera& camera) {
    SDL2pp::Texture& portal = loader.get_texture("portal");
    int src_x = 0, src_y = 638, src_w = 47, src_h = 50;

     int dest_w = src_w *2;
     int dest_h = src_h*2;

    for (const SerializedPortal& p : zone.portals) {
        auto dest = p.cell;
        int cell_x = dest.first*TILE_SIZE - camera.get_rect().x;
        int cell_y = dest.second*TILE_SIZE - camera.get_rect().y;

        int dest_x = cell_x - (dest_w - TILE_SIZE) / 2;
        int dest_y = cell_y - (dest_h - TILE_SIZE);

        visual.draw(portal, src_x, src_y, src_h, src_w, dest_x, dest_y, dest_w, dest_h);
    }

}
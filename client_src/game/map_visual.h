#ifndef MAP_VISUAL_H
#define MAP_VISUAL_H

#include <SDL2pp/SDL2pp.hh>
#include "loader.h"
#include "visual.h"
#include "camera.h"
#include "../../common_src/zone.h"
#include "../../common_src/world_serialization.h"

class MapVisual {
private:
//SDL2pp::Rect rect;
Visual& visual;
Loader& loader;
SerializedWorld world;
const int TILE_SIZE = 16;

const int GRASS_OFFSET[6][2] {
    {0,0},
    {-4, -6},
    {5, 3},
    {-2, 4},
    {6, 6},
    {2,-4}
};

int X_CABIN = 307;
int Y_CABIN = 266;
int W_CABIN = 157;
int H_CABIN = 146;

int W_TREE = 28;
int H_TREE = 43;

int W_PATH = 16;
int H_PATH = 16;

const SerializedZone& get_zone(ZoneId& kind);

//farm
void draw_farm(const SerializedZone& zone, Camera& camera);
void draw_cabin(const SerializedZone& zone, Camera& camera);
void draw_tree(const SerializedZone& zone, Camera& camera);
void draw_path(const SerializedZone& zone, Camera& camera);
void draw_grass(const SerializedZone& zone, Camera& camera);
void draw_walls(const SerializedZone& zone, Camera& camera);
void draw_portals(const SerializedZone& zone, Camera& camera);

//village
void draw_village(const SerializedZone& zone, Camera& camera);

public:
explicit MapVisual(Visual& visual, Loader& loader, SerializedWorld&& world);
void draw_map(ZoneId& zone, Camera& camera);

};

#endif
#ifndef MAP_VISUAL_H
#define MAP_VISUAL_H

#include <SDL2pp/SDL2pp.hh>
#include "loader.h"
#include "visual.h"
#include "camera.h"
#include "../common_src/world_serialization.h"

class MapVisual {
private:
//SDL2pp::Rect rect;
Visual& visual;
Loader& loader;
SerializedWorld world;
const int TILE_SIZE = 32;

void draw_farm(SerializedZone& zone, Camera& camera);

public:
explicit MapVisual(Visual& visual, Loader& loader, SerializedWorld&& world);
void draw_map(Camera& camera);

};

#endif
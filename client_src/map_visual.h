#ifndef MAP_VISUAL_H
#define MAP_VISUAL_H

#include <SDL2pp/SDL2pp.hh>
#include "loader.h"

class MapVisual {
private:


public:
void draw_map(SDL2pp::Renderer& renderer, Loader& loader);

};

#endif
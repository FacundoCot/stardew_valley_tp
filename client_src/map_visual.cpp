#include "map_visual.h"
#include "visual.h"
#include <SDL2pp/SDL2pp.hh>


void MapVisual::draw_map(SDL2pp::Renderer& renderer, Loader& loader) {
    SDL2pp::Texture& map= loader.get_texture("map");

    renderer.Copy(map, SDL2pp::NullOpt, SDL2pp::NullOpt);
    
}
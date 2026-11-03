#ifndef PLAYER_VISUAL_H
#define PLAYER_VISUAL_H

#include <SDL2pp/Renderer.hh>

#include "../common_src/messages.h"
#include "camera.h"
#include "loader.h"

class PlayerVisual {
private:


public:

void draw(SDL2pp::Renderer& renderer, /*Loader& loader,*/ const PlayerState& state, Camera& camera);
};


#endif
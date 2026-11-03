#include <SDL2pp/Renderer.hh>
#include <SDL2pp/Texture.hh>
#include <SDL2pp/Rect.hh>

#include "player_visual.h"
#include "loader.h"
#include "camera.h"
#include "../common_src/messages.h"

using namespace SDL2pp;

void PlayerVisual::draw(Renderer& renderer, /*Loader& loader,*/ const PlayerState& state, Camera& camera) {
    //Texture& player = loader.get_texture("character");
    Rect& r = camera.get_rect();

    int x_screen = state.x - r.x;
    int y_screen = state.y - r.y;

    Rect destino(x_screen, y_screen, 32, 64);


    renderer.SetDrawColor(255,0,0,255);
  

    renderer.FillRect(destino);

    //renderer.Copy(/*player,*/ NullOpt, destino);
}
#include <SDL2pp/Rect.hh>
#include <SDL2pp/Renderer.hh>
#include <SDL2pp/Texture.hh>

#include "../common_src/messages.h"
#include "camera.h"
#include "loader.h"
#include "player_visual.h"

using SDL2pp::Rect;
using SDL2pp::Renderer;

void PlayerVisual::draw(Renderer &renderer,
                        /*Loader& loader,*/ const PlayerState &state,
                        Camera &camera) {
  // Texture& player = loader.get_texture("character");
  const Rect &r = camera.get_rect();

  int x_screen = static_cast<int>(std::round(state.x)) - r.x;
  int y_screen = static_cast<int>(std::round(state.y)) - r.y;

  Rect destino(x_screen, y_screen, 32, 32);

  renderer.SetDrawColor(255, 0, 0, 255);

  renderer.FillRect(destino);

  // renderer.Copy(/*player,*/ NullOpt, destino);
}

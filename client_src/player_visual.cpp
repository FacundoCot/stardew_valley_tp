#include <SDL2pp/Rect.hh>
#include <SDL2pp/Renderer.hh>
#include <SDL2pp/Texture.hh>

#include "../common_src/messages.h"
#include "camera.h"
#include "loader.h"
#include "player_visual.h"
#include "appearance_translator.h"
#include "characterpreview.h"

#include "map_constants.h"

using SDL2pp::Renderer;

PlayerVisual::PlayerVisual(SDL2pp::Renderer& renderer, Loader& loader) : renderer(renderer), loader(loader) {}

void PlayerVisual::draw( const PlayerState &state,
                        Camera &camera, int& it) {

  SDL2pp::Texture& player = loader.get_texture("bodies");

  const SDL2pp::Rect &r = camera.get_rect();

  float x_screen = state.x*TILE_SIZE - r.x;
  float y_screen = state.y*TILE_SIZE - r.y;
  
  CharacterAppearance ap = state.appearance;
  AppearanceTranslator translator(ap);
  
  auto [it_map, inserted] = last_pos.try_emplace(state.player_name, 
    PlayerAnim{
      state.x,
      state.y,
      -100
    });


  PlayerAnim &anim = it_map->second;

  if (!inserted) {
    float dx = std::abs(state.x - anim.x);
    float dy = std::abs(state.y - anim.y);

    if (dx > 0.001f || dy > 0.001f) {
      anim.last_move_it = it;
    }
    anim.x = state.x;
    anim.y = state.y;

  }

  bool is_moving = (it - anim.last_move_it) < 8;

  int step_dx = 0;
  int pos_animation = 0;
  if (is_moving) {
    pos_animation = (it/8) % COUNT_ANIMATIONS;
    step_dx = STEPS[pos_animation];
  }

  Translator::Rect body = translator.body_rect();
  body.x += body.width*step_dx;

  int player_dy =  (is_moving ? PLAYER_DY[pos_animation]*scale : 0);
  
  SDL2pp::Rect dest(x_screen, y_screen, body.width*2, body.height*2);

  SDL2pp::Rect src = to_SDLRect(body);

  renderer.Copy(player, src, dest);

  draw_pants(translator, dest, ap, step_dx);

  if (ap.hair != 0) {
    draw_hair(translator, dest, ap, player_dy);
  }
  draw_shirt(translator, dest, ap, player_dy);
  draw_hat(translator, dest, ap, player_dy);
  
}

void PlayerVisual::draw_hair(AppearanceTranslator& translator, SDL2pp::Rect& body,
                              CharacterAppearance& ap, int& player_dy) {

  SDL2pp::Texture& hair = loader.get_texture("hair");

  Translator::Rect hair_dimensions = translator.hair_rect();

  int dy = HAIR_DY + (ap.body == 1 ? FEMALE_DY : 0);

  SDL2pp::Rect dest(body.x, 
                    body.y + dy*scale + player_dy, 
                    hair_dimensions.width*scale, 
                    hair_dimensions.height*scale);

  SDL2pp::Rect src = to_SDLRect(hair_dimensions);

  uint16_t hair_color = ap.hair_color % COLOR_COUNT;
  hair.SetColorMod(COLORS[hair_color][0], COLORS[hair_color][1], COLORS[hair_color][2]);

  renderer.Copy(hair, src, dest);

}

void PlayerVisual::draw_pants(AppearanceTranslator& translator, SDL2pp::Rect& body, CharacterAppearance& ap, int& step_dx) {
    SDL2pp::Texture& pants = loader.get_texture("pants");

  Translator::Rect pants_dimensions = translator.pants_rect();
  pants_dimensions.x += step_dx * pants_dimensions.width;

  int dy = PANTS_DY + (ap.body == 1 ? FEMALE_DY : 0);

  SDL2pp::Rect dest(body.x, body.y + dy*scale, pants_dimensions.width*scale, pants_dimensions.height*scale);

  SDL2pp::Rect src = to_SDLRect(pants_dimensions);

  uint16_t pants_color = ap.pants_color % COLOR_COUNT;
  pants.SetColorMod(COLORS[pants_color][0], COLORS[pants_color][1], COLORS[pants_color][2]);

  renderer.Copy(pants, src, dest);
}

void PlayerVisual::draw_shirt(AppearanceTranslator& translator, SDL2pp::Rect& body,
                               CharacterAppearance& ap, int& player_dy) {
  SDL2pp::Texture& shirt = loader.get_texture("shirts");

  Translator::Rect shirt_dimension = translator.shirt_rect();

  int dy = SHIRT_DY + (ap.body == 1 ? FEMALE_DY : 0);
  
  SDL2pp::Rect dest(body.x + SHIRT_DX*scale, 
                    body.y + dy*scale + player_dy, 
                    shirt_dimension.width*scale, 
                    shirt_dimension.height*scale);

  SDL2pp::Rect src = to_SDLRect(shirt_dimension);

  renderer.Copy(shirt, src, dest);
}

void PlayerVisual::draw_hat(AppearanceTranslator& translator, SDL2pp::Rect& body, 
                            CharacterAppearance& ap, int& player_dy) {

  if (ap.hat == 0) {
    return;
  }

  SDL2pp::Texture& hat = loader.get_texture("hats");

  Translator::Rect hat_dimension = translator.hat_rect();

  int dy = HAT_DY + (ap.body == 1 ? FEMALE_DY : 0);
  
  SDL2pp::Rect dest(body.x + HAT_DX*scale,
                    body.y + dy*scale + player_dy, 
                    hat_dimension.width*scale, 
                    hat_dimension.height*scale);

  SDL2pp::Rect src = to_SDLRect(hat_dimension);

  renderer.Copy(hat, src, dest);
}

SDL2pp::Rect PlayerVisual::to_SDLRect(Translator::Rect& rect) {
  return SDL2pp::Rect(rect.x, rect.y, rect.width, rect.height);
}



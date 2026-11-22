#ifndef CAMERA_H
#define CAMERA_H

#include "visual.h"
#include <SDL2pp/Rect.hh>
#include "../common_src/messages.h"

class Camera {
private:
  SDL2pp::Rect view;
  const int width_map, height_map;

  // va a tener un personaje

public:
  Camera(/*const int &map_width, const int &map_height*/);

  void update_pos(PlayerState& player, int player_w, int player_h);
  void set_dimensions(int width, int height);
  SDL2pp::Rect &get_rect();
};

#endif

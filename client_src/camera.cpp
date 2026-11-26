#include "camera.h"
#include "../common_src/define.h"
#include "visual.h"
#include "map_constants.h"

Camera::Camera()
    : view(0, 0, 1920, 1080)  {}

void Camera::update_pos(PlayerState& player, int player_w,
                        int player_h) {

  view.x = player.x*TILE_SIZE + player_w / 2 - view.w / 2;
  view.y = (player.y*TILE_SIZE) + player_h / 2 - view.h / 2;

  if (view.w >= width_map) {
    view.x = 0;

  } else {
    if (view.x < 0)
      view.x = 0;
    if (view.x > width_map - view.w) {
      view.x = width_map - view.w;
    }
  }

  if (view.h >= height_map) {
    view.y = 0;

  } else {
    if (view.y < 0)
      view.y = 0;

    if (view.y > height_map - view.h) {
      view.y = height_map - view.h;
    }
  }
}

void Camera::set_dimensions(int width, int height) {
  view.w = width;
  view.h = height;
}

SDL2pp::Rect &Camera::get_rect() { return view; }

void Camera::set_dimensions_map(int& map_w, int& map_h) {
  width_map = map_w;
  height_map = map_h;
}
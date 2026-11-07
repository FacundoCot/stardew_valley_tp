#include "camera.h"
#include "../common_src/define.h"
#include "visual.h"

Camera::Camera(const int &map_width, const int &map_height)
    : view(0, 0, 1920, 1080), width_map(map_width), height_map(map_height) {}

void Camera::update_pos(float player_x, float player_y, int player_w,
                        int player_h) {
  view.x = static_cast<int>(player_x + player_w / 2) - view.w / 2;
  view.y = static_cast<int>(player_y + player_h / 2) - view.h / 2;

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

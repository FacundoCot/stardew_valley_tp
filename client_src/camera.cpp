#include "camera.h"
#include "../common_src/define.h"


Camera::Camera(int& map_width, int& map_height) :  view(0,0, SCREEN_WIDTH, SCREEN_HEIGHT), 
    width_map(map_width),  height_map(map_height) {}


void Camera::update_pos(float& player_x, float& player_y, int player_w, int player_h) {
    view.x = (player_x + player_w / 2) - SCREEN_WIDTH / 2;
    view.y = (player_y + player_h / 2) - SCREEN_HEIGHT / 2;

    if (view.x < 0) {
        view.x = 0;
    }
    if (view.y <  0) {
        view.y = 0;
    }
    if (view.x > width_map - view.w ) {
        view.x = width_map - view.w;
    }
    if (view.y > height_map - view.h) {
        view.y = height_map - view.h;
    }
}

SDL2pp::Rect& Camera::get_rect() {
    return view;
}
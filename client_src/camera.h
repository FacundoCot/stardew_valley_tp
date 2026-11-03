#ifndef CAMERA_H
#define CAMERA_H

#include <SDL2pp/Rect.hh>


class Camera {
private:
SDL2pp::Rect view;
int& width_map, height_map;

//va a tener un personaje

public:
Camera(int& map_width, int& map_height);

void update_pos(float& player_x, float& player_y, int player_w, int player_h);
SDL2pp::Rect& get_rect();

};


#endif 
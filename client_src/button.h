#ifndef BUTTON_H
#define BUTTON_H

#include "visual.h"
#include "loader.h"
#include <SDL2pp/Rect.hh>

class Button {
private:
SDL2pp::Texture& texture;
SDL2pp::Rect button;
bool is_hovered = false;

public:
Button(SDL2pp::Texture& texture, int x, int y, int w, int h);

void draw(Visual& visual) const;
bool is_clicked(int& mouse_x, int&  mouse_y) const;
void update_is_hovered(int mouse_x, int mouse_y);
void set_rect(int x, int y, int w, int h);
};


#endif
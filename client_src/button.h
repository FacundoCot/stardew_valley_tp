#ifndef BUTTON_H
#define BUTTON_H

#include "visual.h"
#include <SDL2pp/Rect.hh>
#include <string>

class Button {
private:
SDL2pp::Rect button;
std::string text;

public:
Button(const std::string& text, Visual& visual, int w, int h);

void draw(Visual& visual) const;

bool is_clicked(int& mouse_x, int&  mouse_y) const;
};


#endif
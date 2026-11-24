#include "button.h"
#include "visual.h"

#include <SDL2pp/Rect.hh>

Button::Button(SDL2pp::Texture& texture,int x, int y, int w, int h) :  texture(texture), 
                                                                    button(x, y, w, h) {}


void Button::draw(Visual& visual) const {
    if (is_hovered) {
        texture.SetColorMod(210, 210, 210);
    } else {
        texture.SetColorMod(255,255,255);
    }

    visual.get_renderer().Copy(texture, SDL2pp::NullOpt, button);

}

bool Button::is_clicked(int& mouse_x, int& mouse_y) const {
    return button.Contains(mouse_x, mouse_y);
}

void Button::update_is_hovered(int mouse_x, int mouse_y) {
    is_hovered = button.Contains(mouse_x, mouse_y);
}

void Button::set_rect(int x, int y, int w, int h) {
    button = SDL2pp::Rect(x, y, w, h);
}
#include "button.h"
#include "visual.h"

#include <SDL2pp/Rect.hh>

Button::Button(const std::string& text, Visual& visual, int w, int h) : button(visual.get_window().GetWidth() / 2 - (w/2), 
                                                                                visual.get_window().GetHeight() / 2 - (h/2), w, h),
                text(text) {}


void Button::draw(Visual& visual) const {
    int mouse_x, mouse_y;
    SDL_GetMouseState(&mouse_x, &mouse_y);

    bool is_hovered = button.Contains(mouse_x, mouse_y);

    visual.draw_button(button, text, is_hovered);

}

bool Button::is_clicked(int& mouse_x, int& mouse_y) const {
    return button.Contains(mouse_x, mouse_y);
}
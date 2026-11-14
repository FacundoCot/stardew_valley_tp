#include <SDL2pp/SDL2pp.hh>
#include <string>

#include "visual.h"
#include <SDL2/SDL.h>

using SDL2pp::Renderer;
using SDL2pp::Window;

Visual::Visual(const std::string &tittle, const int &width, const int &height)
    : sdl(SDL_INIT_VIDEO),
      window(tittle, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width,
             height, SDL_WINDOW_FULLSCREEN | SDL_WINDOW_RESIZABLE),
      renderer(window, -1, SDL_RENDERER_ACCELERATED), ttf(), font("client_src/data/fonts/dayfont.ttf", 18), is_fulscreen(true) {}

Renderer &Visual::get_renderer() { return renderer; }

Window &Visual::get_window() { return window; }

bool Visual::get_is_fulscreen() { return is_fulscreen; }

void Visual::set_is_fulscreen() { is_fulscreen = !is_fulscreen; }

void Visual::clean() { renderer.Clear(); }

void Visual::present() { renderer.Present(); }


void Visual::draw(SDL2pp::Texture& texture, int& src_x, 
             int& src_y, int& src_h, int& src_w, int& dest_x, 
              int& dest_y, int& dest_w, int& dest_h) {
       
       SDL2pp::Rect src_rect(src_x, src_y, src_w, src_h);

       SDL2pp::Rect dest_rect(dest_x, dest_y, dest_w, dest_h);

       renderer.Copy(texture, src_rect, dest_rect);

}

void Visual::draw_rotate(SDL2pp::Texture &texture, int& src_x, 
                         int& src_y, int& src_h, 
                         int& src_w, int& dest_x, 
                         int& dest_y, int& dest_w, int& dest_h,
                         double angle, int& pivot_x, int& pivot_y) {


       SDL2pp::Rect src_rec(src_x, src_y, src_w, src_h);

       SDL2pp::Rect dest_rect (dest_x, dest_y, dest_w, dest_h);

       SDL2pp::Point pivot(pivot_x, pivot_y);

       renderer.Copy(texture, src_rec, dest_rect, angle, pivot, SDL_FLIP_NONE);
}

void Visual::draw_text(const std::string& text, int& x, int& y, uint8_t& r, uint8_t& g, uint8_t& b) {
       if (text.empty()) { return; }

       SDL_Color color = {r, g, b, 255};

       SDL2pp::Surface surface = font.RenderText_Solid(text, color);

       SDL2pp::Texture text_texture(renderer, surface);

       SDL2pp::Rect dest(x, y, surface.GetWidth(), surface.GetHeight());

       renderer.Copy(text_texture, SDL2pp::NullOpt, dest);
}

void Visual::draw_button(const SDL2pp::Rect &rect, const std::string& text, bool is_hovered) {
       renderer.SetDrawBlendMode(SDL_BLENDMODE_NONE);
       if (is_hovered) {
           renderer.SetDrawColor(110, 110, 110, 255);
       } else {
           renderer.SetDrawColor(70, 70, 70, 255);
       }
       
       renderer.FillRect(rect);

       renderer.SetDrawColor(255, 255, 255, 255);
       renderer.DrawRect(rect);

       if (text.empty()) {return;}

       SDL_Color color = {255, 255, 255, 255};
       SDL2pp::Surface text_surface = font.RenderText_Solid(text, color);

       SDL2pp::Texture text_texture(renderer, text_surface);
       
       int center_x = rect.x + (rect.w - text_surface.GetWidth()) / 2;
       int center_y = rect.y + (rect.h - text_surface.GetHeight()) / 2;

       SDL2pp::Rect dest(center_x, center_y, text_surface.GetWidth(), text_surface.GetHeight());

       renderer.Copy(text_texture, SDL2pp::NullOpt, dest);
}

void Visual::draw_black_overlay() {
       renderer.SetDrawBlendMode(SDL_BLENDMODE_BLEND);

       renderer.SetDrawColor(0,0,0, 150);

       renderer.FillRect(SDL2pp::Rect(0,0, renderer.GetOutputWidth(), renderer.GetOutputHeight()));

       renderer.SetDrawBlendMode(SDL_BLENDMODE_NONE);
}

void Visual::draw_text_centered(const std::string& text, uint8_t& r, uint8_t& g, uint8_t& b) {
       if (text.empty()) { return; }

       SDL_Color color = {r, g, b, 255};

       SDL2pp::Surface surface = font.RenderText_Solid(text, color);

       SDL2pp::Texture text_texture(renderer, surface);

       int s_w = surface.GetWidth();
       int s_h = surface.GetHeight();

       SDL2pp::Rect dest(window.GetWidth() / 2 - (s_w / 2), window.GetHeight() / 2 - (s_h / 2), s_w, s_h);

       renderer.Copy(text_texture, SDL2pp::NullOpt, dest);
}
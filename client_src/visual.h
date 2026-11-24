#ifndef VISUAL_H
#define VISUAL_H

#include <SDL2pp/SDL2pp.hh>
#include <string>

class Visual {
private:
  SDL2pp::SDL sdl;
  SDL2pp::Window window;
  SDL2pp::Renderer renderer;
  SDL2pp::SDLTTF ttf;
  SDL2pp::Font font;
  bool is_fulscreen;

public:
  Visual(const std::string &tittle, const int &ancho, const int &alto);
  SDL2pp::Renderer &get_renderer();
  SDL2pp::Window &get_window();
  void draw(SDL2pp::Texture &texture, int& src_x, 
    int& src_y, int& src_h, 
    int& src_w, int& dest_x, 
    int& dest_y, int& dest_w, int& dest_h);

  void draw_rotate(SDL2pp::Texture &texture, int& src_x, 
    int& src_y, int& src_h, 
    int& src_w, int& dest_x, 
    int& dest_y, int& dest_w, int& dest_h,
    double angle, int& pivot_x, int& pivot_y);
  
  void draw_text(const std::string& text, int& x, int& y, uint8_t& r, uint8_t& g, uint8_t& b);
  void draw_black_overlay();
  void draw_text_centered(const std::string& text, uint8_t& r, uint8_t& g, uint8_t& b);

  bool get_is_fulscreen();
  void set_is_fulscreen();
  void clean();
  void present();
};

#endif

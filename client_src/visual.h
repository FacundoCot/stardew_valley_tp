#ifndef VISUAL_H
#define VISUAL_H

#include <SDL2pp/SDL2pp.hh>
#include <string>

class Visual {
private:
  SDL2pp::SDL sdl;
  SDL2pp::Window window;
  SDL2pp::Renderer renderer;
  bool is_fulscreen;

public:
  Visual(const std::string &tittle, const int &ancho, const int &alto);
  SDL2pp::Renderer &get_renderer();
  SDL2pp::Window &get_window();
  bool get_is_fulscreen();
  void set_is_fulscreen();
  void clean();
  void present();
};

#endif

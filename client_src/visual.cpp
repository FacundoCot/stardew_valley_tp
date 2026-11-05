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
      renderer(window, -1, SDL_RENDERER_ACCELERATED), is_fulscreen(true) {}

Renderer &Visual::get_renderer() { return renderer; }

Window &Visual::get_window() { return window; }

bool Visual::get_is_fulscreen() { return is_fulscreen; }

void Visual::set_is_fulscreen() { is_fulscreen = !is_fulscreen; }

void Visual::clean() { renderer.Clear(); }

void Visual::present() { renderer.Present(); }

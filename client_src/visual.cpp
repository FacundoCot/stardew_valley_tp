#include <SDL2pp/SDL2pp.hh>
#include <string>

#include "visual.h"
#include <SDL2/SDL.h>


using namespace SDL2pp;

Visual::Visual(const std::string& tittle, const int& width, const int& height) : 
    sdl(SDL_INIT_VIDEO),
    window(tittle,
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    width, height, SDL_WINDOW_RESIZABLE), renderer(window, -1, SDL_RENDERER_ACCELERATED) {}

SDL2pp::Renderer& Visual::get_renderer() {
    return renderer;
}

void Visual::clean() {
    renderer.Clear();
}

void Visual::present() {
    renderer.Present();
}
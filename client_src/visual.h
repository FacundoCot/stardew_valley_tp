#ifndef VISUAL_H
#define VISUAL_H

#include <SDL2pp/SDL2pp.hh>
#include <string>

class Visual {
private:

SDL2pp::SDL sdl;
SDL2pp::Window window;
SDL2pp::Renderer renderer;


public:

Visual(const std::string& tittle, const int& ancho, const int& alto);
SDL2pp::Renderer& get_renderer();
void clean();
void present();
};


#endif
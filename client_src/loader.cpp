#include "loader.h"
#include <map>
#include <string>
#include <SDL2pp/Surface.hh>
#include <SDL2pp/Texture.hh>
#include <SDL2pp/SDL2pp.hh>
#include <SDL2/SDL.h>



Loader::Loader(SDL2pp::Renderer &renderer_rcv) : renderer(renderer_rcv) {} 


void Loader::load_texture(const std::string& asset_name, const std::string& path) {
    try {
        SDL2pp::Surface surface(path);
        assets.emplace(asset_name, SDL2pp::Texture(renderer, surface));
    } catch(const std::exception &e) {
        //lanzar excepcion propia
    }

}

SDL2pp::Texture& Loader::get_texture(const std::string& name) {
    auto t = assets.find(name);

    if (t == assets.end()) {
        //lanzar excepcion custom
    } 

    return t->second;
}
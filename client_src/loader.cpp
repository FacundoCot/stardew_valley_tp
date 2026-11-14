#include "loader.h"
#include "../common_src/custom_error.h"
#include <map>
#include <string>
#include <format>
#include <SDL2pp/Surface.hh>
#include <SDL2pp/Texture.hh>
#include <SDL2pp/SDL2pp.hh>




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
        throw AssetNotFound(std::format("Error at Loader: Asset with name: {} is not found.", name));
    } 

    return t->second;
}
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
        std::cerr << e.what() << "\n";
    }

}

SDL2pp::Texture& Loader::get_texture(const std::string& name) {
    auto t = assets.find(name);

    if (t == assets.end()) {
        throw AssetNotFound("Error at Loader: Asset with name: " + name + " is not found.");
    } 

    return t->second;
}

void Loader::load_everything() {
    load_texture("clock","client_src/data/clock.png");
    load_texture("Inventory", "client_src/data/InventorySlots.png");
    load_texture("pickaxe", "client_src/data/pickaxe.png");
    load_texture("shovel", "client_src/data/shovel.png");
    load_texture("sword", "client_src/data/sword.png");
    load_texture("watering_can", "client_src/data/water.png");
    load_texture("bed", "client_src/data/Single_Bed.png");
    load_texture("shirts", "client_src/data/assets/Characters/StardewValley/shirts.png");
    load_texture("pants", "client_src/data/assets/Characters/StardewValley/pants.png");
    load_texture("hats", "client_src/data/assets/Characters/StardewValley/hats.png");
    load_texture("hair", "client_src/data/assets/Characters/StardewValley/hairstyles.png");
    load_texture("bodies", "client_src/data/assets/Characters/StardewValley/bodies.png");
}



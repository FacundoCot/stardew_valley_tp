#include "loader.h"
#include "../common_src/custom_error.h"
#include <map>
#include <string>
#include <format>
#include <SDL2pp/Surface.hh>
#include <SDL2pp/Texture.hh>
#include <SDL2pp/SDL2pp.hh>




Loader::Loader(SDL2pp::Renderer &renderer_rcv) : renderer(renderer_rcv) {load_everything();} 


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
    load_texture("ground", "client_src/data/assets/Farm/Environment/ground.png");
    load_texture("grass", "client_src/data/grass.png");
    load_texture("cabin", "client_src/data/cabin_final.png");
    load_texture("tree1", "client_src/data/assets/Misc/tree2/tree1.png");
    load_texture("rock", "client_src/data/assets/Misc/rock.png");
    load_texture("portal", "client_src/data/assets/Farm/Environment/Tilesets_Furniture.png");
    load_texture("menu_in_game", "client_src/data/menu/woodTemplate.png");
    load_texture("exit", "client_src/data/menu/salir.png");
    load_texture("btn_confirm_day", "client_src/data/next_day_button.jpg");
    load_texture("enable", "client_src/data/menu/enable_button.jpg");
}

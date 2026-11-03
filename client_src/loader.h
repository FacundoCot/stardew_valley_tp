#ifndef LOADER_H
#define LOADER_H


#include <SDL2pp/SDL2pp.hh>
#include <map>

class Loader {
private:

std::unordered_map<std::string, SDL2pp::Texture> assets;
SDL2pp::Renderer& renderer;

public:
explicit Loader(SDL2pp::Renderer &renderer_rcv); 
void load_texture(const std::string& asset_name, const std::string& path);
SDL2pp::Texture& get_texture(const std::string& name);

};

#endif 
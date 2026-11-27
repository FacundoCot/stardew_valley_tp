#ifndef WORLD_LOADER_H
#define WORLD_LOADER_H

#include <filesystem>
#include <memory>
#include <string>

#include "../common_src/world.h"

class WorldLoader {
public:
    static std::shared_ptr<const World> load(const std::string& map_name,
                                             const std::filesystem::path& maps_directory);
};

#endif

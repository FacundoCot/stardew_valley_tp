#ifndef WORLD_CATALOG_H
#define WORLD_CATALOG_H

#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>

#include "../common_src/world.h"

class WorldCatalog {
private:
    std::mutex mtx;
    std::map<std::string, std::function<std::shared_ptr<const World>()>> builders;
    std::map<std::string, std::shared_ptr<const World>> worlds;

public:
    WorldCatalog();
    std::shared_ptr<const World> get(const std::string& name);
};

#endif

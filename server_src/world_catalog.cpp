#include "world_catalog.h"

#include <stdexcept>

#include "../common_src/default_world.h"

WorldCatalog::WorldCatalog():
        builders{{"por_defecto", build_default_world}} {}

std::shared_ptr<const World> WorldCatalog::get(const std::string& name) {
    std::lock_guard<std::mutex> lock(mtx);
    const auto existing = worlds.find(name);
    if (existing != worlds.end()) {
        return existing->second;
    }
    const auto builder = builders.find(name);
    if (builder == builders.end()) {
        throw std::invalid_argument("mundo desconocido: " + name);
    }
    auto world = builder->second();
    worlds.emplace(name, world);
    return world;
}

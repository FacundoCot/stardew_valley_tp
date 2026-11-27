#include "world_catalog.h"

#include <filesystem>
#include <stdexcept>
#include "world_loader.h"

WorldCatalog::WorldCatalog():
        builders{} {}

std::shared_ptr<const World> WorldCatalog::get(const std::string& name) {
    std::lock_guard<std::mutex> lock(mtx);
    std::string canonical = name;
    if (name == "por_defecto") {
        canonical = "map_1";
    }
    if (canonical.empty() || canonical == "." || canonical == ".." ||
        canonical.find('/') != std::string::npos ||
        canonical.find('\\') != std::string::npos ||
        canonical.find("..") != std::string::npos ||
        std::filesystem::path(canonical).is_absolute()) {
        throw std::invalid_argument("nombre de mapa invalido: " + name);
    }

    const auto existing = worlds.find(canonical);
    if (existing != worlds.end()) {
        return existing->second;
    }

    std::filesystem::path maps_directory;
    std::filesystem::path current = std::filesystem::current_path();
    for (std::filesystem::path candidate = current;
         !candidate.empty(); candidate = candidate.parent_path()) {
        const auto maps = candidate / "editor" / "maps";
        if (std::filesystem::is_directory(maps)) {
            maps_directory = maps;
            break;
        }
        if (candidate == candidate.parent_path()) {
            break;
        }
    }
    if (maps_directory.empty()) {
        throw std::runtime_error("No se encontro editor/maps para el mapa '" +
                                 canonical + "'");
    }

    auto world = WorldLoader::load(canonical, maps_directory);
    worlds.emplace(canonical, world);
    return world;
}

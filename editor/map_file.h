#ifndef MAP_FILE_H
#define MAP_FILE_H

#include <map>
#include <ostream>
#include <set>
#include <string>
#include <vector>

#include <QPixmap>
#include <toml++/toml.hpp>

#include "map_scene.h"


const std::string LAYER_IDS[LAYER_COUNT] = {"ground", "walls", "objects", "decor"};
const bool LAYER_SOLID[LAYER_COUNT] = {false, true, true, false};

class MapFile {
private:
    void write_zone(std::ostream& out, const MapScene* scene, const std::string& kind, int level, 
                    int map_columns, int map_rows, std::set<std::string>& sheets) const;

    void write_layer(std::ostream& out, const MapScene* scene, int layer, int map_columns, int map_rows,
                     std::set<std::string>& sheets) const;

    bool load_sheets(const toml::table& root, const std::string& assets_path,
                     std::map<std::string, QPixmap>& sheets, std::string& error) const;

    bool load_zone(const toml::table& zone, const std::map<std::string, QPixmap>& sheets, int map_columns, 
                    int map_rows, const std::vector<MapScene*>& scenes, std::string& error) const;

    bool load_tile(const toml::table& tile, const std::map<std::string, QPixmap>& sheets,
                   int map_columns, int map_rows, MapScene* scene, std::string& error) const;

    int zone_index(const std::string& kind, int level, int zone_count) const;

    int layer_index(const std::string& id) const;

public:
    // false si falló algo al guardar
    bool save(const std::string& path, const std::string& map_name, int map_columns,
              int map_rows, const std::vector<MapScene*>& scenes) const;

    // false si falló al cargar
    bool load(const std::string& path, const std::string& assets_path, int map_columns,
              int map_rows, const std::vector<MapScene*>& scenes,
              std::string& error) const;
};

#endif

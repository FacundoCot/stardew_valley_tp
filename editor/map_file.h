#ifndef MAP_FILE_H
#define MAP_FILE_H

#include <ostream>
#include <set>
#include <string>
#include <vector>

#include "map_scene.h"


const std::string LAYER_IDS[LAYER_COUNT] = {"ground", "walls", "objects", "decor"};
const bool LAYER_SOLID[LAYER_COUNT] = {false, true, true, false};

class MapFile {
private:
    void write_zone(std::ostream& out, const MapScene* scene, const std::string& kind, int level, 
                    int map_columns, int map_rows, std::set<std::string>& sheets) const;

    void write_layer(std::ostream& out, const MapScene* scene, int layer, int map_columns, int map_rows,
                     std::set<std::string>& sheets) const;

public:
    // false si falló algo al guardar
    bool save(const std::string& path, const std::string& map_name, int map_columns,
              int map_rows, const std::vector<MapScene*>& scenes) const;
};

#endif

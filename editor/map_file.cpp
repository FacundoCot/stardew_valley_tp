#include "map_file.h"

#include <fstream>

#include <QGraphicsItem>
#include <QString>

#include "common_src/define.h"

void MapFile::write_layer(std::ostream& writer, const MapScene* scene, int layer,
                          int map_columns, int map_rows,
                          std::set<std::string>& sheets) const {
    for (int row = 0; row < map_rows; ++row) {
        for (int column = 0; column < map_columns; ++column) {
            const QGraphicsItem* item = scene->tile_item(layer, column, row);
            if (item == nullptr) {
                continue;
            }
            const std::string sheet = item->data(TILE_DATA_SHEET).toString().toStdString();
            sheets.insert(sheet);

            writer << "[[zone.tile]]\n";
            writer << "x = " << column << "\n";
            writer << "y = " << row << "\n";
            writer << "sheet = \"" << sheet << "\"\n";
            writer << "col = " << item->data(TILE_DATA_COLUMN).toInt() << "\n";
            writer << "row = " << item->data(TILE_DATA_ROW).toInt() << "\n";
            writer << "layer = \"" << LAYER_IDS[layer] << "\"\n\n";
        }
    }
}

void MapFile::write_zone(std::ostream& writer, const MapScene* scene,
                         const std::string& kind, int level, int map_columns,
                         int map_rows, std::set<std::string>& sheets) const {
    writer << "[[zone]]\n";
    writer << "kind = \"" << kind << "\"\n";
    writer << "level = " << level << "\n\n";

    for (int layer = 0; layer < LAYER_COUNT; ++layer) {
        write_layer(writer, scene, layer, map_columns, map_rows, sheets);
    }
}

bool MapFile::save(const std::string& path, const std::string& map_name,
                   int map_columns, int map_rows,
                   const std::vector<MapScene*>& scenes) const {
    std::ofstream writer(path);
    if (!writer) {
        return false;
    }

    writer << "name = \"" << map_name << "\"\n";
    writer << "tile_size = " << TILE_SIZE << "\n";
    writer << "width = " << map_columns << "\n";
    writer << "height = " << map_rows << "\n\n";

    for (int layer = 0; layer < LAYER_COUNT; ++layer) {
        writer << "[[layer]]\n";
        writer << "name = \"" << LAYER_IDS[layer] << "\"\n";
        writer << "solid = " << (LAYER_SOLID[layer] ? "true" : "false") << "\n\n";
    }

    // cargando las sheets usadas a medida que escribe las zonas
    std::set<std::string> used_sheets;
    write_zone(writer, scenes[0], "farm", 0, map_columns, map_rows, used_sheets);
    write_zone(writer, scenes[1], "village", 0, map_columns, map_rows, used_sheets);
    for (size_t i = 2; i < scenes.size(); ++i) {
        write_zone(writer, scenes[i], "cave", static_cast<int>(i) - 1, map_columns, map_rows, used_sheets);
    }

    for (const std::string& sheet: used_sheets) {
        writer << "[[sheet]]\n";
        writer << "path = \"" << sheet << "\"\n";
        writer << "tile_w = " << TILE_SIZE << "\n";
        writer << "tile_h = " << TILE_SIZE << "\n\n";
    }

    return writer.good();
}

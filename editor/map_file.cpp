#include "map_file.h"

#include <fstream>

#include <QGraphicsItem>
#include <QString>

#include "common_src/define.h"

void MapFile::write_layer(std::ostream& writer, const MapScene* scene, int layer,
                          std::set<std::string>& sheets) const {
    for (int row = 0; row < scene->row_count(); ++row) {
        for (int column = 0; column < scene->column_count(); ++column) {
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

void MapFile::write_elements(std::ostream& writer, const MapScene* scene) const {
    for (int row = 0; row < scene->row_count(); ++row) {
        for (int column = 0; column < scene->column_count(); ++column) {
            const QGraphicsItem* item = scene->element_item(column, row);
            if (item == nullptr) {
                continue;
            }
            const int type = item->data(ELEMENT_DATA_TYPE).toInt();

            writer << "[[zone.element]]\n";
            writer << "type = \"" << ELEMENT_TYPES[type].id << "\"\n";
            writer << "x = " << column << "\n";
            writer << "y = " << row << "\n\n";
        }
    }
}

void MapFile::write_zone(std::ostream& writer, const MapScene* scene,
                         const std::string& kind, int level,
                         std::set<std::string>& sheets) const {
    writer << "[[zone]]\n";
    writer << "kind = \"" << kind << "\"\n";
    writer << "level = " << level << "\n";
    writer << "width = " << scene->column_count() << "\n";
    writer << "height = " << scene->row_count() << "\n\n";

    for (int layer = 0; layer < LAYER_COUNT; ++layer) {
        write_layer(writer, scene, layer, sheets);
    }
    write_elements(writer, scene);
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
    write_zone(writer, scenes[ZONE_FARM], "farm", 0, used_sheets);
    write_zone(writer, scenes[ZONE_VILLAGE], "village", 0, used_sheets);
    for (int level = 1; level <= CAVE_LEVELS; ++level) {
        write_zone(writer, scenes[ZONE_FIRST_CAVE + level - 1], "cave", level, used_sheets);
    }
    write_zone(writer, scenes[ZONE_HOUSE], "house", 0, used_sheets);
    write_zone(writer, scenes[ZONE_COMMUNITY_CENTER], "community_center", 0, used_sheets);

    for (const std::string& sheet: used_sheets) {
        writer << "[[sheet]]\n";
        writer << "path = \"" << sheet << "\"\n";
        writer << "tile_w = " << TILE_SIZE << "\n";
        writer << "tile_h = " << TILE_SIZE << "\n\n";
    }

    return writer.good();
}

int MapFile::zone_index(const std::string& kind, int level) const {
    int index = -1;

    if (kind == "farm") {
        index = ZONE_FARM;
    } else if (kind == "village") {
        index = ZONE_VILLAGE;
    } else if (kind == "cave") {
        if (level >= 1 && level <= CAVE_LEVELS) {
            index = ZONE_FIRST_CAVE + level - 1;
        }
    } else if (kind == "house") {
        index = ZONE_HOUSE;
    } else if (kind == "community_center") {
        index = ZONE_COMMUNITY_CENTER;
    }

    return index;
}

int MapFile::layer_index(const std::string& id) const {
    for (int layer = 0; layer < LAYER_COUNT; ++layer) {
        if (LAYER_IDS[layer] == id) {
            return layer;
        }
    }
    return -1;
}

int MapFile::element_index(const std::string& id) const {
    for (int type = 0; type < ELEMENT_COUNT; ++type) {
        if (ELEMENT_TYPES[type].id == id) {
            return type;
        }
    }
    return -1;
}

bool MapFile::load_sheets(const toml::table& root, const std::string& assets_path,
                          std::map<std::string, QPixmap>& sheets,
                          std::string& error) const {
    const toml::array* list = root["sheet"].as_array();
    if (list == nullptr) {
        return true;
    }
    for (const toml::node& node : *list) {
        const toml::table* sheet = node.as_table();
        if (sheet == nullptr) {
            error = "Error: [[sheet]] mal formado";
            return false;
        }

        const std::string name = (*sheet)["path"].value_or(std::string());
        if ((*sheet)["tile_w"].value_or(0) != TILE_SIZE || (*sheet)["tile_h"].value_or(0) != TILE_SIZE) {
            error = "Error: tamaño erroneo de sheet";
            return false;
        }

        const std::string path = assets_path + "/" + name;
        const QPixmap pixmap(QString::fromStdString(path));
        if (pixmap.isNull()) {
            error = "Error: fallo al cargar sheet " + path;
            return false;
        }
        sheets[name] = pixmap;
    }
    return true;
}

bool MapFile::load_tile(const toml::table& tile, const std::map<std::string, QPixmap>& sheets,
                        MapScene* scene, std::string& error) const {
    const int x = tile["x"].value_or(-1);
    const int y = tile["y"].value_or(-1);
    const int col = tile["col"].value_or(-1);
    const int row = tile["row"].value_or(-1);
    const std::string sheet_name = tile["sheet"].value_or(std::string());
    const std::string layer_id = tile["layer"].value_or(std::string());

    const int layer = layer_index(layer_id);
    if (layer < 0) {
        error = "Error: capa desconocida";
        return false;
    }
    if (x < 0 || y < 0 || x >= scene->column_count() || y >= scene->row_count()) {
        error = "Error: tile fuera de la zona";
        return false;
    }

    const auto found = sheets.find(sheet_name); // iterador
    if (found == sheets.end()) {
        error = "Error: sheet desconocida para tile";
        return false;
    }
    
    const QPixmap& sheet = found->second;
    if (col < 0 || row < 0 || (col + 1) * TILE_SIZE > sheet.width() 
        || (row + 1) * TILE_SIZE > sheet.height()) {
        error = "Error: tile fuera de hoja";
        return false;
    }

    // hago mismo corte como el de sheet scene
    const QPixmap piece = sheet.copy(col * TILE_SIZE, row * TILE_SIZE, TILE_SIZE, TILE_SIZE);
    scene->paint_cell(layer, x, y, piece, sheet_name, col, row);
    return true;
}

bool MapFile::load_element(const toml::table& element, MapScene* scene,
                           std::string& error) const {
    const int x = element["x"].value_or(-1);
    const int y = element["y"].value_or(-1);
    const std::string type_id = element["type"].value_or(std::string());

    const int type = element_index(type_id);
    if (type < 0) {
        error = "Error: elemento desconocido";
        return false;
    }
    if (x < 0 || y < 0 || x >= scene->column_count() || y >= scene->row_count()) {
        error = "Error: elemento fuera de la zona";
        return false;
    }
    scene->place_element(type, x, y);
    return true;
}

bool MapFile::load_zone(const toml::table& zone, const std::map<std::string, QPixmap>& sheets,
                        int map_columns, int map_rows, const std::vector<MapScene*>& scenes,
                        std::string& error) const {
    const std::string kind = zone["kind"].value_or(std::string());
    const int level = zone["level"].value_or(0);
    const int index = zone_index(kind, level);
    if (index < 0) {
        error = "Error: zona desconocida";
        return false;
    }
    MapScene* scene = scenes[static_cast<size_t>(index)];

    // asumo estándar si el archivo no lo tiene presente
    if (zone["width"].value_or(map_columns) != scene->column_count() ||
        zone["height"].value_or(map_rows) != scene->row_count()) {
        error = "Error: tamaño de zona distinto al del editor";
        return false;
    }

    const toml::array* tiles = zone["tile"].as_array();
    if (tiles != nullptr) {
        for (const toml::node& node : *tiles) {
            const toml::table* tile = node.as_table();
            if (tile == nullptr) {
                error = "Error: [[zone.tile]] mal formado";
                return false;
            }
            if (!load_tile(*tile, sheets, scene, error)) {
                return false;
            }
        }
    }

    const toml::array* elements = zone["element"].as_array();
    if (elements != nullptr) {
        for (const toml::node& node : *elements) {
            const toml::table* element = node.as_table();
            if (element == nullptr) {
                error = "Error: [[zone.element]] mal formado";
                return false;
            }
            if (!load_element(*element, scene, error)) {
                return false;
            }
        }
    }
    return true;
}

bool MapFile::load(const std::string& path, const std::string& assets_path,
                   int map_columns, int map_rows,
                   const std::vector<MapScene*>& scenes, std::string& error) const {

    toml::table root;
    try {
        root = toml::parse_file(path); // carga todo en memoria
    } catch (const toml::parse_error& parse_error) {
        error = std::string(parse_error.description()) + " (linea " +
                std::to_string(parse_error.source().begin.line) + ")";
        return false;
    }

    if (root["tile_size"].value_or(0) != TILE_SIZE ||
        root["width"].value_or(0) != map_columns || root["height"].value_or(0) != map_rows) {
        error = "Error: valores del mapa no coinciden";
        return false;
    }

    std::map<std::string, QPixmap> sheets;
    if (!load_sheets(root, assets_path, sheets, error)) {
        return false;
    }

    for (MapScene* scene: scenes) {
        scene->clear_tiles();
    }

    const toml::array* zones = root["zone"].as_array();
    if (zones == nullptr) {
        return true;  // mapa sin zonas pintadas
    }
    for (const toml::node& node : *zones) {
        const toml::table* zone = node.as_table();
        if (zone == nullptr) {
            error = "Error: [[zone]] mal formado";
            return false;
        }
        if (!load_zone(*zone, sheets, map_columns, map_rows, scenes, error)) {
            return false;
        }
    }
    return true;
}

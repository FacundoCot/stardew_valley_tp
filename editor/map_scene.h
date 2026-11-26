#ifndef MAP_SCENE_H
#define MAP_SCENE_H

#include <vector>

#include <QGraphicsScene>
#include <QPixmap>
#include <QStringList>
#include <QPixmap>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsItem>
#include <QPainter>

const int LAYER_GROUND = 0;
const int LAYER_WALLS = 1;
const int LAYER_OBJECTS = 2;
const int LAYER_DECOR = 3;
const int LAYER_COUNT = 4;
const int LAYER_ELEMENTS = LAYER_COUNT;

const int TILE_DATA_SHEET = 0; // rutas
const int TILE_DATA_COLUMN = 1;
const int TILE_DATA_ROW = 2;

const int ELEMENT_DATA_TYPE = 0; 

class MapScene: public QGraphicsScene {
    Q_OBJECT

public:
    enum Tool { PAINT, ERASE };

private:
    int columns;
    int rows;
    Tool tool;
    int layer; // capa activa

    QPixmap tile; // tile activo (el ultimo que se eligió)
    std::string tile_sheet;
    int tile_column;
    int tile_row;

    int selection_size;
    int press_column;
    int press_row;

    // celdas de cada capa
    std::vector<std::vector<QGraphicsItem*>> layer_cells;

    int element_type; // elegido
    std::vector<QGraphicsItem*> element_cells; // aparte de layer_cells

    int to_cell(qreal coordinate) const;
    void apply_at(const QPointF& position);

    void erase_cell(int layer_index, int column, int row); // individuales 

    void paint_selection(int column, int row); // seleccion completa
    void erase_selection(int column, int row);

    void erase_element(int column, int row);

protected:
    void drawBackground(QPainter* painter, const QRectF& rect) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;

public:
    MapScene(int map_columns, int map_rows, QObject* parent = nullptr);

    void set_tool(Tool new_tool);
    void set_layer(int new_layer);
    void set_tile(const QPixmap& new_tile, std::string sheet_name, int row, int column);
    QStringList layer_names() const;

    QGraphicsItem* tile_item(int layer_index, int column, int row) const;

    void paint_cell(int layer_index, int column, int row, const QPixmap& piece,
                    const std::string& sheet_name, int piece_column, int piece_row);

    void place_element(int type_index, int column, int row);

    void set_element_type(int new_type);
    QGraphicsItem* element_item(int column, int row) const;

    int column_count() const;
    int row_count() const;

    void clear_tiles();
};

#endif
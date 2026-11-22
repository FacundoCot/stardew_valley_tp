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

const int TILE_DATA_SHEET = 0; // rutas
const int TILE_DATA_COLUMN = 1;
const int TILE_DATA_ROW = 2;

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

    int to_cell(qreal coordinate) const;
    void apply_at(const QPointF& position);

    void paint_cell(int column, int row, const QPixmap& piece, int piece_column, int piece_row);
    void erase_cell(int column, int row); // individuales 

    void paint_selection(int column, int row); // seleccion completa
    void erase_selection(int column, int row);

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
};

#endif
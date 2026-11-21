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

    // celdas de cada capa
    std::vector<std::vector<QGraphicsItem*>> layer_cells;

    int to_cell(qreal coordinate) const;
    void apply_at(const QPointF& position);
    void paint_cell(int column, int row);
    void erase_cell(int column, int row);

protected:
    void drawBackground(QPainter* painter, const QRectF& rect) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;

public:
    MapScene(int map_columns, int map_rows, QObject* parent = nullptr);

    void set_tool(Tool new_tool);
    void set_layer(int new_layer);
    void set_tile(const QPixmap& new_tile);
    QStringList layer_names() const;
};

#endif
#ifndef EDITOR_MAP_SCENE_H
#define EDITOR_MAP_SCENE_H

#include <vector>
#include <QStringList>
#include <QColor>
#include <QGraphicsScene>

class QGraphicsItem;
class QGraphicsSceneMouseEvent;
class QPainter;

const int LAYER_GROUND = 0;
const int LAYER_WALLS = 1;
const int LAYER_OBJECTS = 2;
const int LAYER_DECOR = 3;
const int LAYER_COUNT = 4;

enum Tool { PAINT, ERASE };

class MapScene: public QGraphicsScene {
    Q_OBJECT

private:
    int columns;
    int rows;
    Tool tool;
    int layer;  // capa activa

    std::vector<std::vector<QGraphicsItem*>> layer_cells;

    int to_cell(qreal coordinate) const;
    QColor layer_color(int layer_index) const;
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
    QStringList layer_names() const;
};

#endif
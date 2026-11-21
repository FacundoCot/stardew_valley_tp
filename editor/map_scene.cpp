#include "map_scene.h"

#include <algorithm>
#include <cmath>

#include <QGraphicsPixmapItem>
#include <QLineF>
#include <QPen>

#include "common_src/define.h" 

MapScene::MapScene(int map_columns, int map_rows, QObject* parent):
        QGraphicsScene(parent),
        columns(map_columns),
        rows(map_rows),
        tool(PAINT),
        layer(LAYER_GROUND),
        layer_cells(LAYER_COUNT,
                    std::vector<QGraphicsItem*>(map_columns * map_rows, nullptr)) {
    setSceneRect(0, 0, columns * TILE_SIZE, rows * TILE_SIZE);
}

void MapScene::set_tool(Tool new_tool) {
    tool = new_tool;
}

void MapScene::set_layer(int new_layer) {
    layer = new_layer;
}

void MapScene::set_tile(const QPixmap& new_tile) {
    tile = new_tile;
}

QStringList MapScene::layer_names() const {
    return {"ground", "walls", "objects", "decor"};
}

int MapScene::to_cell(qreal coordinate) const {
    return static_cast<int>(std::floor(coordinate / TILE_SIZE));
}

void MapScene::drawBackground(QPainter* painter, const QRectF& rect) {
    painter->fillRect(rect, QColor(20, 20, 20));
    painter->setPen(QPen(QColor(255, 255, 255, 40)));

    const int first_column = std::max(0, to_cell(rect.left()));
    const int last_column = std::min(columns, to_cell(rect.right()) + 1);
    const int first_row = std::max(0, to_cell(rect.top()));
    const int last_row = std::min(rows, to_cell(rect.bottom()) + 1);

    for (int column = first_column; column <= last_column; ++column) {
        const qreal x = column * TILE_SIZE;
        painter->drawLine(QLineF(x, 0, x, rows * TILE_SIZE));
    }
    for (int row = first_row; row <= last_row; ++row) {
        const qreal y = row * TILE_SIZE;
        painter->drawLine(QLineF(0, y, columns * TILE_SIZE, y));
    }
}

void MapScene::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        apply_at(event->scenePos());
    }
}

void MapScene::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
    if (event->buttons().testFlag(Qt::LeftButton)) {
        apply_at(event->scenePos());
    }
}

void MapScene::apply_at(const QPointF& position) {
    const int column = to_cell(position.x());
    const int row = to_cell(position.y());
    if (column < 0 || row < 0 || column >= columns || row >= rows) {
        return;
    }
    if (tool == PAINT) {
        paint_cell(column, row);
    } else {
        erase_cell(column, row);
    }
}

void MapScene::paint_cell(int column, int row) {
    if (tile.isNull()) {
        return;
    }
    erase_cell(column, row);  // reemplazo si ya había algo en la capa

    const int index = row * columns + column;
    QGraphicsPixmapItem* item = addPixmap(tile);
    item->setPos(column * TILE_SIZE, row * TILE_SIZE);
    item->setZValue(layer);
    layer_cells[layer][index] = item;
}

void MapScene::erase_cell(int column, int row) {
    const int index = row * columns + column;

    QGraphicsItem* item = layer_cells[layer][index];
    if (item == nullptr) {
        return;
    }

    removeItem(item);
    delete item;
    layer_cells[layer][index] = nullptr;
}
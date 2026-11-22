#include "map_scene.h"

#include <algorithm>
#include <cmath>

#include <QGraphicsPixmapItem>
#include <QLineF>
#include <QPen>
#include <QString>

#include "common_src/define.h" 

MapScene::MapScene(int map_columns, int map_rows, QObject* parent):
        QGraphicsScene(parent),
        columns(map_columns),
        rows(map_rows),
        tool(PAINT),
        layer(LAYER_GROUND),
        tile_column(0),
        tile_row(0),
        selection_size(1),
        press_column(0),
        press_row(0),
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

void MapScene::set_tile(const QPixmap& new_tile, std::string sheet_name, int row, int column) {
    tile = new_tile;
    tile_sheet = sheet_name;
    tile_column = column;
    tile_row = row;
    selection_size = std::max(1, new_tile.width() / TILE_SIZE);  // ajusto segun el valor de sheet scene
}

QStringList MapScene::layer_names() const {
    return {"Pisos", "Paredes", "Objetos", "Decoraciones"};
}

QGraphicsItem* MapScene::tile_item(int layer_index, int column, int row) const {
    return layer_cells[layer_index][row * columns + column];
}

int MapScene::to_cell(qreal coordinate) const {
    return static_cast<int>(std::floor(coordinate / TILE_SIZE));
}

void MapScene::drawBackground(QPainter* painter, const QRectF& rect) {
    painter->fillRect(rect, QColor(20, 20, 20));
    painter->setPen(QPen(QColor(255, 255, 255, 40)));

    const int first_column = std::max(0, to_cell(rect.left()));
    const int last_column = std::min(columns, to_cell(rect.right()));
    const int first_row = std::max(0, to_cell(rect.top()));
    const int last_row = std::min(rows, to_cell(rect.bottom()));

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
        press_column = to_cell(event->scenePos().x());
        press_row = to_cell(event->scenePos().y());
        apply_at(event->scenePos());
    }
}

void MapScene::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
    if (event->buttons().testFlag(Qt::LeftButton)) {
        const int column = to_cell(event->scenePos().x());
        const int row = to_cell(event->scenePos().y());
        const int y_distance = (row - press_row);
        const int x_distance = (column - press_column);
        if (x_distance % selection_size == 0 && y_distance % selection_size == 0) {
            apply_at(event->scenePos());
        }
    }
}

void MapScene::apply_at(const QPointF& position) {
    const int column = to_cell(position.x());
    const int row = to_cell(position.y());
    if (column < 0 || row < 0 || column >= columns || row >= rows) {
        return;
    }
    if (tool == PAINT) {
        paint_selection(column, row);
    } else {
        erase_selection(column, row);
    }
}

void MapScene::paint_selection(int column, int row) {
    if (tile.isNull()) {
        return;
    }
    for (int dy = 0; dy < selection_size; ++dy) {
        for (int dx = 0; dx < selection_size; ++dx) {
            const int cell_column = column + dx;
            const int cell_row = row + dy;
            if (cell_column >= columns || cell_row >= rows) {
                continue;  // esta parte del selection caería afuear del mapa
            }

            const QPixmap piece = tile.copy(dx * TILE_SIZE, dy * TILE_SIZE, TILE_SIZE, TILE_SIZE);
            paint_cell(cell_column, cell_row, piece, tile_column + dx, tile_row + dy);
        }
    }
}

void MapScene::erase_selection(int column, int row) {
    for (int dy = 0; dy < selection_size; ++dy) {
        for (int dx = 0; dx < selection_size; ++dx) {
            const int cell_column = column + dx;
            const int cell_row = row + dy;
            if (cell_column < columns && cell_row < rows) {
                erase_cell(cell_column, cell_row);
            }
        }
    }
}

void MapScene::paint_cell(int column, int row, const QPixmap& piece,
                          int piece_column, int piece_row) {
    erase_cell(column, row);  // reemplazo si ya había algo en la capa

    const int index = row * columns + column;
    QGraphicsPixmapItem* item = addPixmap(piece);

    // datos en relación al mapa
    item->setPos(column * TILE_SIZE, row * TILE_SIZE);
    item->setZValue(layer);

    // los datos del sprite en relación a su sheet
    item->setData(TILE_DATA_SHEET, QString::fromStdString(tile_sheet));
    item->setData(TILE_DATA_COLUMN, piece_column);
    item->setData(TILE_DATA_ROW, piece_row);

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

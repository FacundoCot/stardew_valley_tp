#include "sheet_scene.h"

#include <cmath>

#include <QBrush>
#include <QColor>
#include <QPen>
#include <QString>

#include "map_definitions.h"

SheetScene::SheetScene(QObject* parent):
        QGraphicsScene(parent),
        sheet_item(nullptr),
        selection_item(addRect(0, 0, TILE_SIZE, TILE_SIZE, QPen(QColor(0, 255, 0)), QBrush(Qt::NoBrush))),
        selection_size(1),
        selected_column(-1),
        selected_row(-1)
        {
    selection_item->setZValue(1);
    selection_item->setVisible(false);
}

bool SheetScene::load_sheet(const std::string& path) {
    const QPixmap loaded(QString::fromStdString(path));
    if (loaded.isNull()) {
        return false;
    }
    sheet = loaded;

    if (sheet_item != nullptr) {
        removeItem(sheet_item);
        delete sheet_item;
    }

    sheet_item = addPixmap(sheet);
    sheet_item->setZValue(0);
    setSceneRect(0, 0, sheet.width(), sheet.height());

    select_tile(0, 0);
    return true;
}

void SheetScene::select_tile(int column, int row) {
    const int columns = sheet.width() / TILE_SIZE;
    const int rows = sheet.height() / TILE_SIZE;

    if (sheet.isNull() || column < 0 || row < 0 || column + selection_size > columns ||
        row + selection_size > rows) {
        return;  // o click fuera de la imagen o el selection size no entra
    }
    selected_column = column;
    selected_row = row;

    int side = selection_size * TILE_SIZE;
    selection_item->setRect(0, 0, side, side);
    selection_item->setPos(column * TILE_SIZE, row * TILE_SIZE);
    selection_item->setVisible(true);

    const QPixmap tile = sheet.copy(column * TILE_SIZE, row * TILE_SIZE, side, side);
    emit tile_selected(tile, row, column);
}

void SheetScene::set_selection_size(int size) {
    selection_size = size;
    select_tile(selected_column, selected_row);
}

void SheetScene::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        int column = static_cast<int>(std::floor(event->scenePos().x() / TILE_SIZE));
        int row = static_cast<int>(std::floor(event->scenePos().y() / TILE_SIZE));
        select_tile(column, row);
    }
}

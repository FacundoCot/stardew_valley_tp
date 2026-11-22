#ifndef SHEET_SCENE_H
#define SHEET_SCENE_H

#include <string>

#include <QGraphicsScene>
#include <QPixmap>

#include <QGraphicsPixmapItem>
#include <QGraphicsRectItem>
#include <QGraphicsSceneMouseEvent>

class SheetScene: public QGraphicsScene {
    Q_OBJECT

private:
    QPixmap sheet;
    QGraphicsPixmapItem* sheet_item; // la sheet como item para el scene
    QGraphicsRectItem* selection_item;
    int selection_size;

    int selected_column;
    int selected_row;

    void select_tile(int column, int row);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

signals:
    void tile_selected(const QPixmap& tile, int row, int column);

public:
    explicit SheetScene(QObject* parent = nullptr);

    bool load_sheet(const std::string& path);

    void set_selection_size(int size);
};

#endif

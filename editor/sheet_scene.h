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

    void select_tile(int column, int row);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

signals:
    void tile_selected(const QPixmap& tile);

public:
    explicit SheetScene(QObject* parent = nullptr);

    bool load_sheet(const std::string& path);
};

#endif

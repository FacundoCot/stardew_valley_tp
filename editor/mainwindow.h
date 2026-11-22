#ifndef EDITOR_MAINWINDOW_H
#define EDITOR_MAINWINDOW_H

#include <string>
#include <vector>

#include <QMainWindow>
#include <QPixmap>
#include "map_scene.h"
#include "sheet_scene.h"

const int CAVE_LEVELS = 10;

// totalmente provisional
const int MAP_COLUMNS = 180;
const int MAP_ROWS = 120;

const std::string ASSETS_PATH = "client_src/data/assets";

namespace Ui {
 class MainWindow;
}

class MainWindow: public QMainWindow {
    Q_OBJECT

private:
    Ui::MainWindow* ui;
    std::vector<MapScene*> scenes;
    SheetScene* sheet_scene;
    std::vector<std::string> sheet_names;

    void fill_sheet_list();

private slots:
    void change_zone(int index);
    void change_layer(int index);
    void change_tool(bool painting);

    void change_sheet(int index);
    void change_tile(const QPixmap& tile, int row, int column);
    void change_selection_size(int index);

    void save_map();

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;
};

#endif

#ifndef EDITOR_MAINWINDOW_H
#define EDITOR_MAINWINDOW_H

#include <vector>

#include <QMainWindow>

class MapScene;

namespace Ui {
class MainWindow;
}

class MainWindow: public QMainWindow {
    Q_OBJECT

private:
    Ui::MainWindow* ui;
    std::vector<MapScene*> scenes;

private slots:
    void change_zone(int index);
    void change_layer(int index);
    void change_tool(bool painting);

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;
};

#endif

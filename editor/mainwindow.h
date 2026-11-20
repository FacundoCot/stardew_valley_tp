#ifndef EDITOR_MAINWINDOW_H
#define EDITOR_MAINWINDOW_H

#include <vector>
#include <QGraphicsScene>
#include <QMainWindow>

const int CAVE_LEVELS = 10;

namespace Ui {
    class MainWindow;
}

class MainWindow: public QMainWindow {
    Q_OBJECT

private:
    Ui::MainWindow* ui;

    std::vector<QGraphicsScene*> scenes;

private slots:
    void change_zone(int index);
    void change_layer(/*const QString& layer*/);
    void change_tool(bool painting);

public:
    explicit MainWindow(QWidget* parent = nullptr);
    virtual ~MainWindow();
};

#endif

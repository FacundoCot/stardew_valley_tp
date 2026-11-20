#include "mainwindow.h"

#include <cstddef>

#include <QComboBox>
#include <QGraphicsView>
#include <QRadioButton>
#include <QString>

#include "map_scene.h"
#include "ui_mainwindow.h"

const int CAVE_LEVELS = 10;

// provisorio, debería ser configurable.
// de momento tomo mismos datos que el define de common
const int MAP_COLUMNS = 180;
const int MAP_ROWS = 120;

MainWindow::MainWindow(QWidget* parent):
        QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    const int zone_count = 2 + CAVE_LEVELS;
    for (int i = 0; i < zone_count; ++i) {
        scenes.push_back(new MapScene(MAP_COLUMNS, MAP_ROWS, this));
    }

    ui->mapGraphicsView->setScene(scenes.front());

    ui->zoneComboBox->addItem("Granja");
    ui->zoneComboBox->addItem("Pueblo");
    for (int level = 1; level <= CAVE_LEVELS; ++level) {
        const std::string name = "Caverna " + std::to_string(level);
        ui->zoneComboBox->addItem(QString::fromStdString(name));
    }

    ui->layerComboBox->addItems(scenes.front()->layer_names());

    connect(ui->zoneComboBox, &QComboBox::currentIndexChanged, this,
            &MainWindow::change_zone);
    connect(ui->layerComboBox, &QComboBox::currentIndexChanged, this,
            &MainWindow::change_layer);
    connect(ui->paintRadioButton, &QRadioButton::toggled, this,
            &MainWindow::change_tool);
}

MainWindow::~MainWindow() {
    delete ui;
}


void MainWindow::change_zone(int index) {
    if (index < 0) {
        return;
    }
    ui->mapGraphicsView->setScene(scenes.at(static_cast<size_t>(index)));
}


void MainWindow::change_layer(int index) {
    if (index < 0) {
        return;
    }
    for (MapScene* scene: scenes) {
        scene->set_layer(index);
    }
}

void MainWindow::change_tool(bool painting) {
    for (MapScene* scene: scenes) {
        if (painting) {
            scene->set_tool(Tool::PAINT);
        } else {
            scene->set_tool(Tool::ERASE);
        }
        // mas tools (?)
    }
}
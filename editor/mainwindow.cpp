#include "mainwindow.h"

#include <QComboBox>
#include <QGraphicsView>
#include <QRadioButton>
#include <QString>

#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget* parent):
        QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    ui->zoneCombo->addItem("Granja");
    ui->zoneCombo->addItem("Pueblo");
    for (int level = 1; level <= CAVE_LEVELS; ++level) {
        ui->zoneCombo->addItem("Caverna " + QString::number(level));
    }
    ui->layerCombo->addItem("ground");
    ui->layerCombo->addItem("walls");
    ui->layerCombo->addItem("objects");
    ui->layerCombo->addItem("decor");

    for (int i = 0; i < ui->zoneCombo->count(); ++i) {
        scenes.push_back(new QGraphicsScene(this));
    }
    ui->mapView->setScene(scenes.front());

    connect(ui->zoneCombo, &QComboBox::currentIndexChanged, this,
            &MainWindow::change_zone);
    connect(ui->layerCombo, &QComboBox::currentTextChanged, this,
            &MainWindow::change_layer);
    connect(ui->toolPaint, &QRadioButton::toggled, this,
            &MainWindow::change_tool);
}

void MainWindow::change_zone(int index) {
    if (index < 0) {
        return;
    }
    ui->mapView->setScene(scenes.at(index));
}

void MainWindow::change_layer(/* const QString& layer */) {
    // cambiar altura que se está editando
}

void MainWindow::change_tool(bool painting) {
    if (painting) {
        // usar pincel
    } else {
        // usar borrador
    } 
    // faltarían meter más como el bucket
}

MainWindow::~MainWindow(){ delete ui; };

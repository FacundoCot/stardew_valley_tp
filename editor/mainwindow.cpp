#include "mainwindow.h"

#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <string>

#include <QComboBox>
#include <QGraphicsView>
#include <QRadioButton>
#include <QStatusBar>
#include <QString>

#include "ui_mainwindow.h"

const int CAVE_LEVELS = 10;

// totalmente provisional
const int MAP_COLUMNS = 180;
const int MAP_ROWS = 120;

const std::string ASSETS_PATH = "client_src/data/assets";

MainWindow::MainWindow(QWidget* parent):
        QMainWindow(parent), ui(new Ui::MainWindow), sheet_scene(nullptr) {
    ui->setupUi(this);

    const int zone_count = 2 + CAVE_LEVELS;
    for (int i = 0; i < zone_count; ++i) {
        scenes.push_back(new MapScene(MAP_COLUMNS, MAP_ROWS, this));
    }
    change_zone(0);

    ui->zoneComboBox->addItem("Granja");
    ui->zoneComboBox->addItem("Pueblo");
    for (int level = 1; level <= CAVE_LEVELS; ++level) {
        std::string cave_level = "Caverna" + std::to_string(level);
        ui->zoneComboBox->addItem(QString::fromStdString(cave_level));
    }

    ui->layerComboBox->addItems(scenes.front()->layer_names());

    
    sheet_scene = new SheetScene(this);
    ui->sheetGraphicsView->setScene(sheet_scene);
    connect(sheet_scene, &SheetScene::tile_selected, this, &MainWindow::change_tile);

    fill_sheet_list();
    if (!sheet_names.empty()) {
        change_sheet(0);
    }

    connect(ui->zoneComboBox, &QComboBox::currentIndexChanged, this,
            &MainWindow::change_zone);
    connect(ui->layerComboBox, &QComboBox::currentIndexChanged, this,
            &MainWindow::change_layer);
    connect(ui->paintRadioButton, &QRadioButton::toggled, this,
            &MainWindow::change_tool);
    connect(ui->sheetComboBox, &QComboBox::currentIndexChanged, this,
            &MainWindow::change_sheet);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::fill_sheet_list() {
    std::vector<std::string> folders;
    folders.push_back("Farm");
    folders.push_back("Market");
    folders.push_back("Caverns/Environment");
    folders.push_back("Misc");

    for (const std::string& folder : folders) {
        const std::string folder_path = ASSETS_PATH + "/" + folder;
        if (!std::filesystem::is_directory(folder_path)) {
            continue;
        }

        for (const std::filesystem::directory_entry& entry : std::filesystem::recursive_directory_iterator(folder_path)) {
            if (entry.is_regular_file() && entry.path().extension() == ".png") {
                // extraigo ASSETS_PATH
                sheet_names.push_back(std::filesystem::relative(entry.path(), ASSETS_PATH).string());
            }
        }
    }
    std::sort(sheet_names.begin(), sheet_names.end());

    if (sheet_names.empty()) {
        ui->statusbar->showMessage("No se pudo encontrar assets.");
        return;
    }

    for (const std::string& name: sheet_names) {
        ui->sheetComboBox->addItem(QString::fromStdString(name));
    }
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
            scene->set_tool(MapScene::PAINT);
        } else {
            scene->set_tool(MapScene::ERASE);
        }
    }
}

void MainWindow::change_sheet(int index) {
    if (index < 0) {
        return;
    }
    const std::string path = ASSETS_PATH + "/" + sheet_names.at(index);
    if (!sheet_scene->load_sheet(path)) {
        ui->statusbar->showMessage(QString::fromStdString("No se pudo cargar " + path));
    }
}

void MainWindow::change_tile(const QPixmap& tile) {
    for (MapScene* scene: scenes) {
        scene->set_tile(tile);
    }
}
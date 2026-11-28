#include "mainwindow.h"

#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <string>

#include <QAction>
#include <QComboBox>
#include <QFileDialog>
#include <QGraphicsView>
#include <QMessageBox>
#include <QRadioButton>
#include <QStatusBar>
#include <QString>

#include "map_file.h"
#include "map_validator.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent):
        QMainWindow(parent), ui(new Ui::MainWindow), sheet_scene(nullptr) {
    ui->setupUi(this);

    for (int i = 0; i < ZONE_COUNT; ++i) {
        if (i == ZONE_HOUSE) {
            scenes.push_back(new MapScene(HOUSE_COLUMNS, HOUSE_ROWS, this));
        } else if (i == ZONE_COMMUNITY_CENTER) {
            scenes.push_back(new MapScene(CENTER_COLUMNS, CENTER_ROWS, this));
        } else {
            scenes.push_back(new MapScene(MAP_COLUMNS, MAP_ROWS, this));
        }
    }
    change_zone(0);

    ui->zoneComboBox->addItem("Granja");
    ui->zoneComboBox->addItem("Pueblo");
    for (int level = 1; level <= CAVE_LEVELS; ++level) {
        std::string cave_level = "Caverna" + std::to_string(level);
        ui->zoneComboBox->addItem(QString::fromStdString(cave_level));
    }
    ui->zoneComboBox->addItem("Casa");
    ui->zoneComboBox->addItem("Centro comunitario");

    ui->layerComboBox->addItems(scenes.front()->layer_names());

    for (int i = 0; i < ELEMENT_COUNT; ++i) {
        ui->elementTypeComboBox->addItem(QString::fromStdString(ELEMENT_TYPES[i].name));
    }

    
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
    connect(ui->selectionSizeComboBox, &QComboBox::currentIndexChanged, this,
            &MainWindow::change_selection_size);
    connect(ui->elementTypeComboBox, &QComboBox::currentIndexChanged, this,
            &MainWindow::change_element_type);
    connect(ui->openAction, &QAction::triggered, this, &MainWindow::open_map);
    connect(ui->saveAction, &QAction::triggered, this, &MainWindow::save_map);
    connect(ui->validateAction, &QAction::triggered, this, &MainWindow::validate_map);
}

void MainWindow::fill_sheet_list() {
    std::vector<std::string> folders;
    folders.push_back("Farm");
    folders.push_back("Market");
    folders.push_back("Caverns");
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

    bool elements = (index == LAYER_ELEMENTS);
    ui->elementTypeComboBox->setEnabled(elements);
    ui->sheetComboBox->setEnabled(!elements);
    ui->sheetGraphicsView->setEnabled(!elements);
    ui->selectionSizeComboBox->setEnabled(!elements);
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

void MainWindow::change_tile(const QPixmap& tile, int row, int column) {
    std::string& sheet_name = sheet_names.at(ui->sheetComboBox->currentIndex());
    for (MapScene* scene : scenes) {
        scene->set_tile(tile, sheet_name, row, column);
    }
}

void MainWindow::change_selection_size(int index) {
    if (index < 0) {
        return;
    }

    sheet_scene->set_selection_size(index + 1);
}

void MainWindow::change_element_type(int index) {
    if (index < 0) {
        return;
    }
    for (MapScene* scene: scenes) {
        scene->set_element_type(index);
    }
}

void MainWindow::open_map() {
    const QString chosen = QFileDialog::getOpenFileName(this, "Abrir mapa", "", "Mapas (*.toml)");
    if (chosen.isEmpty()) {
        return; // el usuario cancela
    }

    std::string file_path = chosen.toStdString();
    
    MapFile map_file;
    std::string error;
    if (map_file.load(file_path, ASSETS_PATH, MAP_COLUMNS, MAP_ROWS, scenes, error)) {
        ui->statusbar->showMessage(QString::fromStdString("Mapa abierto: " + file_path));
    } else {
        ui->statusbar->showMessage(QString::fromStdString("No se pudo abrir " + file_path + ": " + error));
    }
}

void MainWindow::save_map() {
    QString chosen = QFileDialog::getSaveFileName(this, "Guardar mapa", "", "Mapas (*.toml)");
    if (chosen.isEmpty()) {
        return; // el usuario cancelo
    }
    if (!chosen.endsWith(".toml")) {
        chosen += ".toml";
    }

    std::string file_path = chosen.toStdString();
    std::string map_name = std::filesystem::path(file_path).stem().string();

    MapFile map_file;
    if (map_file.save(file_path, map_name, MAP_COLUMNS, MAP_ROWS, scenes)) {
        ui->statusbar->showMessage(QString::fromStdString("Mapa guardado en " + file_path));
    } else {
        ui->statusbar->showMessage(QString::fromStdString("No se pudo guardar " + file_path));
    }
}

void MainWindow::validate_map() {
    MapValidator validator;
    const std::vector<std::string> problems = validator.validate(scenes);

    if (problems.empty()) {
        ui->statusbar->showMessage("El mapa es válido.");
        QMessageBox::information(this, "Validar mapa", "El mapa es válido.");
    } else {
        QString problems_box;
        for (const std::string& problem : problems) {
            problems_box += QString::fromStdString(problem) + "\n";
        }

        QMessageBox box(QMessageBox::Warning, "Validar mapa",
                        QString("El mapa tiene " + QString::number(problems.size()) + " problema(s)."), 
                        QMessageBox::Ok, this);
        box.setDetailedText(problems_box); // desplegable para los problemas
        box.exec();
    }
}

MainWindow::~MainWindow() { delete ui; }

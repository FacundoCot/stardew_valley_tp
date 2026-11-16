#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <iostream>

MainWindow::MainWindow(Queue<ServerMessage>& resp, Queue<ClientMessage>& comm, QWidget* parent) : QMainWindow(parent), responses(resp),
 commands(comm), ui(new Ui::MainWindow), timer(new QTimer(this)) {
    ui->setupUi(this);
    
    ui->stackedWidget->setCurrentIndex(MAIN);
    ui->titleLabel->setPixmap(QPixmap("client_src/data/menu/upscaledMainTitle.png"));
    set_mainwindow_connections();
    set_character_connections();
}

void MainWindow::set_mainwindow_connections() {
    connect(timer, &QTimer::timeout,
            this, &MainWindow::timer_create_response);

    connect(ui->backButton1, &QPushButton::clicked,
            this, &MainWindow::backButton_clicked);

    connect(ui->backButton2, &QPushButton::clicked,
            this, &MainWindow::backButton_clicked);

    connect(ui->backButton3, &QPushButton::clicked,
            this, &MainWindow::backButton_clicked);
}

void MainWindow::set_character_connections(){
    connect(ui->bodyLeft, &QPushButton::clicked, ui->characterPreview, &CharacterPreview::prev_body);
    connect(ui->bodyRight, &QPushButton::clicked, ui->characterPreview, &CharacterPreview::next_body);
    connect(ui->hairLeft,  &QPushButton::clicked, ui->characterPreview, &CharacterPreview::prev_hair);
    connect(ui->hairRight, &QPushButton::clicked, ui->characterPreview, &CharacterPreview::next_hair);
    connect(ui->shirtLeft,  &QPushButton::clicked, ui->characterPreview, &CharacterPreview::prev_shirt);
    connect(ui->shirtRight, &QPushButton::clicked, ui->characterPreview, &CharacterPreview::next_shirt);
    connect(ui->pantsLeft,  &QPushButton::clicked, ui->characterPreview, &CharacterPreview::prev_pants);
    connect(ui->pantsRight, &QPushButton::clicked, ui->characterPreview, &CharacterPreview::next_pants);
    connect(ui->hatLeft,  &QPushButton::clicked, ui->characterPreview, &CharacterPreview::prev_hat);
    connect(ui->hatRight, &QPushButton::clicked, ui->characterPreview, &CharacterPreview::next_hat);
    connect(ui->hairColorLeft,  &QPushButton::clicked, ui->characterPreview, &CharacterPreview::prev_haircolor);
    connect(ui->hairColorRight, &QPushButton::clicked, ui->characterPreview, &CharacterPreview::next_haircolor);
    connect(ui->pantsColorLeft,  &QPushButton::clicked, ui->characterPreview, &CharacterPreview::prev_pantscolor);
    connect(ui->pantsColorRight, &QPushButton::clicked, ui->characterPreview, &CharacterPreview::next_pantscolor);
}

bool MainWindow::has_exited() { return exited; }

void MainWindow::closeEvent(QCloseEvent* event){
    if(!game_started) {
        exited = true;
    }
    event->accept();
}

void MainWindow::on_createButton_clicked(){
    customization_menu(MenuStyle::CREATE);
}

void MainWindow::on_continueButton_clicked(){
    ui->stackedWidget->setCurrentIndex(CONTINUE);
    // consultar a servidor por partidas relacionadas a este cliente
}

void MainWindow::on_joinButton_clicked(){
    ui->stackedWidget->setCurrentIndex(JOIN);
}

void MainWindow::on_exitButton_clicked(){
    exited = true;
    this->close();
}

void MainWindow::on_submitCodeButton_clicked(){
    std::string code = ui->codeBox->text().toStdString();
    /*
    if (code.length() != 4){
        QMessageBox::warning(this, "Error", "Inserte un codigo de 4 valores");
        return;
    }
    */
    this->join_code = code;

    // en realidad habría que hacer esto:
    // enviar input por receiver a servidor
    // obtener respuesta
    // si es valido ui->stackedWidget->setCurrentIndex(CUSTOMIZATION_CREATE);
    // pero de momento:

    customization_menu(MenuStyle::JOIN);
}

void MainWindow::on_startCreateButton_clicked(){
    ClientMessage msg;
    
    msg.player_name = ui->playerNameEdit->text().trimmed().toStdString();
    if(msg.player_name.empty()){
        QMessageBox::warning(this, "Error", "Inserte un nombre de jugador");
        return;
    }
    
    if (style == MenuStyle::CREATE){
        msg.type = ActionType::CREATE_GAME;
        
        msg.game_name = ui->farmNameEdit->text().trimmed().toStdString();   
        if(msg.game_name.empty()){
            QMessageBox::warning(this, "Error", "Inserte un nombre de granja");
            return;
        }

        msg.game_name_len = msg.game_name.size();
    } else {
        msg.type = ActionType::JOIN_GAME;

        msg.game_code = join_code;
    }

    msg.player_name_len = msg.player_name.size();
    msg.appearance = ui->characterPreview->get_appearance();
    
    commands.push(msg);
    
    ui->startCreateButton->setEnabled(false);
    timer->start(20);
}

void MainWindow::backButton_clicked(){
    ui->stackedWidget->setCurrentIndex(MAIN);
}

void MainWindow::timer_create_response() {
    ServerMessage res;

    if(!responses.try_pop(res)){
        return;
    }
    timer->stop();
    ui->startCreateButton->setEnabled(true);

    if(res.type == MessageType::ERROR){
        if (style == MenuStyle::JOIN) {
            QMessageBox::warning(this, "Error", "No se pudo unir. Posible código invalido.");
            ui->stackedWidget->setCurrentIndex(JOIN);
        } else {
            QMessageBox::warning(this, "Error", "No se pudo crear la partida");
        }
    } else {
        std::cout << res.game_code << std::endl;
        game_started = true;
        this->close();
    }
}

void MainWindow::customization_menu(MenuStyle new_style) {
    style = new_style;
    bool if_create = (new_style == MenuStyle::CREATE);

    ui->farmNameEdit->setVisible(if_create);
    ui->farmNameLabel->setVisible(if_create);
    ui->stackedWidget->setCurrentIndex(CUSTOM_CREATE);
}

MainWindow::~MainWindow() { delete ui; }
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>

MainWindow::MainWindow(Queue<ServerMessage>& resp, Queue<ClientMessage>& comm, QWidget* parent) : QMainWindow(parent), responses(resp),
 commands(comm), ui(new Ui::MainWindow), timer(new QTimer(this)) {
    ui->setupUi(this);
    
    ui->stackedWidget->setCurrentIndex(MAIN);
    ui->titleLabel->setPixmap(QPixmap("client_src/menuSprites/upscaledMainTitle.png"));
    connect(timer, &QTimer::timeout, this, &MainWindow::timer_create_response);
    connect(ui->createButton, &QPushButton::clicked, this, &MainWindow::on_click_createButton);
    connect(ui->continueButton, &QPushButton::clicked, this, &MainWindow::on_click_continueButton);
    connect(ui->joinButton, &QPushButton::clicked, this, &MainWindow::on_click_joinButton);
    connect(ui->exitButton, &QPushButton::clicked, this, &MainWindow::on_click_exitButton);
    connect(ui->submitCodeButton, &QPushButton::clicked, this, &MainWindow::on_click_submitCodeButton);
    connect(ui->backButton1, &QPushButton::clicked, this, &MainWindow::on_click_backButton);
    connect(ui->backButton2, &QPushButton::clicked, this, &MainWindow::on_click_backButton);
    connect(ui->backButton3, &QPushButton::clicked, this, &MainWindow::on_click_backButton);
    connect(ui->startCreateButton, &QPushButton::clicked, this, &MainWindow::on_click_startCreateButton);
}

bool MainWindow::has_exited() { return exited; }

void MainWindow::on_click_createButton(){
    ui->stackedWidget->setCurrentIndex(CUSTOM_CREATE);
}

void MainWindow::on_click_continueButton(){
    ui->stackedWidget->setCurrentIndex(CONTINUE);
    // consultar a servidor por partidas relacionadas a este cliente
}

void MainWindow::on_click_joinButton(){
    ui->stackedWidget->setCurrentIndex(JOIN);
}

void MainWindow::on_click_exitButton(){
    exited = true;
    this->close();
}

void MainWindow::on_click_submitCodeButton(){
    // obtener input de caja de texto
    QString code = ui->codeBox->text();
    
    // enviar input por receiver a servidor
    // obtener respuesta

    // si es valido ui->stackedWidget->setCurrentIndex(CUSTOMIZATION_CREATE); 
    // inhabilitar seccion de customizar granja
}

void MainWindow::on_click_startCreateButton(){
    ClientMessage msg;
    msg.type = ActionType::CREATE_GAME;
    msg.game_name = ui->farmNameEdit->text().toStdString();
    if(msg.game_name.empty()){
        QMessageBox::warning(this, "Error", "Inserte un nombre de granja");
        return;
    }

    msg.player_name = ui->playerNameEdit->text().toStdString();
    if(msg.game_name.empty()){
        QMessageBox::warning(this, "Error", "Inserte un nombre de jugador");
        return;
    }

    msg.game_name_len = msg.game_name.size();
    msg.player_name_len = msg.player_name.size();
    msg.appearance = CharacterAppearance{};
    
    commands.push(msg);
    
    ui->startCreateButton->setEnabled(false);
    timer->start(20);
}

void MainWindow::on_click_backButton(){
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
        QMessageBox::warning(this, "Error", "Campos inválidos");
        return;
    } else {
        this->close();
    }
 }

MainWindow::~MainWindow() { delete ui; }
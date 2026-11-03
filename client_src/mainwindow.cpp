#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(Receiver& rcvr, Sender& sndr, QWidget* parent) : QMainWindow(parent), receiver(rcvr), sender(sndr), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    
    ui->stackedWidget->setCurrentIndex(MAIN);
    ui->titleLabel->setPixmap(QPixmap("client_src/menuSprites/upscaledMainTitle.png"));
    connect(ui->createButton, &QPushButton::clicked, this, &MainWindow::on_click_createButton);
    connect(ui->continueButton, &QPushButton::clicked, this, &MainWindow::on_click_continueButton);
    connect(ui->joinButton, &QPushButton::clicked, this, &MainWindow::on_click_joinButton);
    connect(ui->exitButton, &QPushButton::clicked, this, &MainWindow::on_click_exitButton);
    connect(ui->backButton1, &QPushButton::clicked, this, &MainWindow::on_click_backButton);
}

bool MainWindow::has_exited() { return exited; }

void MainWindow::on_click_createButton(){
    ui->stackedWidget->setCurrentIndex(CUSTOMIZATION_CREATE);
}

void MainWindow::on_click_continueButton(){
    ui->stackedWidget->setCurrentIndex(CONTINUE);
}

void MainWindow::on_click_joinButton(){
    ui->stackedWidget->setCurrentIndex(CUSTOMIZATION_JOIN);
}

void MainWindow::on_click_exitButton(){
    exited = true;
    this->close();
}

void MainWindow::on_click_backButton(){
    ui->stackedWidget->setCurrentIndex(MAIN);
}

MainWindow::~MainWindow() { delete ui; }
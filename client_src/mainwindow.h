#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "../common_src/queue.h"
#include <QTimer>
#include "../common_src/messages.h"
#include <QCloseEvent>
#include "characterpreview.h"

enum Pages {
    MAIN,
    CUSTOM_CREATE,
    CONTINUE,
    JOIN,
    NAME
};

namespace Ui {
    class MainWindow;
}

enum class MenuStyle { 
    CREATE, 
    JOIN 
};

class MainWindow : public QMainWindow {
    Q_OBJECT

    private:
        Queue<ServerMessage>& responses;
        Queue<ClientMessage>& commands;
        Ui::MainWindow* ui;
        QTimer* timer;
        bool exited = false;
        bool game_started = false;
        MenuStyle style = MenuStyle::CREATE;
        std::string join_code;
        std::string username;
        
        void customization_menu(MenuStyle new_style);
        void closeEvent(QCloseEvent* event) override;
        void set_mainwindow_connections();
        void set_character_connections();
    private slots:
        void on_submitUsernameButton_clicked();
        void on_createButton_clicked();
        void on_continueButton_clicked();
        void on_joinButton_clicked();
        void on_exitButton_clicked();
        void on_submitCodeButton_clicked();
        void on_startCreateButton_clicked();
        void backButton_clicked();
        void timer_create_response();
    public:
        explicit MainWindow(Queue<ServerMessage>& responses, Queue<ClientMessage>& commands, QWidget* parent = nullptr);

        bool has_exited();

        virtual ~MainWindow();
};

#endif
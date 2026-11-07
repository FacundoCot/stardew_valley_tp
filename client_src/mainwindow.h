#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "../common_src/queue.h"
#include <QTimer>
#include "../common_src/messages.h"
#include <QCloseEvent>

enum Pages {
    MAIN,
    CUSTOM_CREATE,
    CONTINUE,
    JOIN
};

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

    private:
        Queue<ServerMessage>& responses;
        Queue<ClientMessage>& commands;
        Ui::MainWindow* ui;
        QTimer* timer;
        bool exited = false;

        void closeEvent(QCloseEvent* event) override;

    private slots:
        void on_click_createButton();
        void on_click_continueButton();
        void on_click_joinButton();
        void on_click_exitButton();
        void on_click_submitCodeButton();
        void on_click_backButton();
        void on_click_startCreateButton();
        void timer_create_response();
    public:
        explicit MainWindow(Queue<ServerMessage>& responses, Queue<ClientMessage>& commands, QWidget* parent = nullptr);

        bool has_exited();

        virtual ~MainWindow();
};

#endif
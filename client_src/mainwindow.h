#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "receiver.h"
#include "sender.h"
#include <QMainWindow>

enum Pages {
    MAIN,
    CONTINUE,
    CUSTOMIZATION_CREATE,
    CUSTOMIZATION_JOIN
};

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

    private:
        Receiver& receiver;
        Sender& sender;
        Ui::MainWindow* ui;
        bool exited = false;

    private slots:
        void on_click_createButton();
        void on_click_continueButton();
        void on_click_joinButton();
        void on_click_exitButton();
        void on_click_backButton();

    public:
        explicit MainWindow(Receiver& receiver, Sender& sender, QWidget* parent = nullptr);

        bool has_exited();

        virtual ~MainWindow();
};

#endif
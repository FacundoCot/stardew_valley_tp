#include "menu.h"

#include <QApplication>
#include "mainwindow.h"
#include "../common_src/character_appearance.h"

Menu::Menu(ClientProtocol& protocol) : protocol(protocol), receiver(responses, protocol),
    sender(commands, protocol) 
    {}


bool Menu::run(int argc, char* argv[]){
    receiver.start();
    sender.start();

    return main_menu(argc, argv);
}

bool Menu::main_menu(int argc, char *argv[]) {
    QApplication app(argc, argv);

    MainWindow menu(responses, commands);
    menu.show();

    app.exec();

    return menu.has_exited();
}

void Menu::stop(){
    responses.close();
    commands.close();
    receiver.stop();
    sender.stop();
}

Menu::~Menu(){
    stop();
    receiver.join();
    sender.join();
}
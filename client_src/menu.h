#ifndef MENU_H
#define MENU_H

#include "client_protocol.h"
#include "receiver.h"
#include "sender.h"
#include "../common_src/queue.h"

class Menu {
    public:
        explicit Menu(ClientProtocol& protocol);

        bool run(int argc, char* argv[]);
        
        ~Menu();
    private:
        Queue<ServerMessage> responses;
        Queue<ClientMessage> commands;

        ClientProtocol& protocol;
        Receiver receiver;
        Sender sender;

        bool main_menu(int argc, char *argv[]);

        void stop();
};

#endif
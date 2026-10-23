#ifndef SERVER_H
#define SERVER_H

#include "../common_src/socket.h"
#include "../common_src/queue.h"
#include "menu_games_monitor.h"
#include "acceptor.h"

class Server {
private:
    Queue<std::shared_ptr<ClientAction>> actions;
    MenuGamesMonitor menu_games_monitor;
    Acceptor acceptor;


public:
    explicit Server(Socket&& skt);
    void run();
};

#endif

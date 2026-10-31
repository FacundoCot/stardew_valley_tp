#ifndef ACCEPTOR_H
#define ACCEPTOR_H

#include <memory>
#include <vector>

#include "../common_src/socket.h"
#include "../common_src/thread.h"
#include "../common_src/queue.h"

#include "client_handler.h"
#include "client_action.h"
#include "menu_games_monitor.h"
#include "client_registry.h"

class Acceptor: public Thread {
private:
    Socket skt;

    int next_client_id = 1;

    // vector de clienthandlers los cuales todos pertenecen a acceptor
    // por lo tanto cuando se destruya acceptor por RAII se van a destruir
    // todos los clientes, sin necesidad de hacer delete client
    // unique_ptr = objeto tiene un solo dueño
    std::vector<std::unique_ptr<ClientHandler>> clients;

    MenuGamesMonitor& menu_games_monitor;
    ClientRegistry& registry;

    void clean();
    void reap();

public:
    explicit Acceptor(Socket&& skt,
                      MenuGamesMonitor& menu_games_monitor,
                      ClientRegistry& registry);

    void run() override;
    void stop() override;
};

#endif

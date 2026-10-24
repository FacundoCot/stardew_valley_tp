#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include <memory>

#include "../common_src/socket.h"
#include "../common_src/messages.h"
#include "../common_src/queue.h"
#include "client_receiver.h"
#include "client_sender.h"
#include "client_action.h"
#include "../common_src/thread.h"
#include "menu_games_monitor.h"

class ClientHandler: public Thread {
private:
    Socket skt;
    Queue<ServerMessage> outgoing;

    MenuGamesMonitor& menu_games_monitor;

    std::shared_ptr<Game> game;
    std::string player_name;

    // uso unique_ptr para poder inicializar joiner y reciever recien cuando
    // el cliente se haya unido a una partida
    std::unique_ptr<ClientReceiver> receiver;
    std::unique_ptr<ClientSender> sender;

    bool reciever_started = false;
    bool sender_started = false;

    void send_menu_result(const MenuResult& result, ActionType action, ServerProtocol& protocol);

public:
    explicit ClientHandler(Socket&& skt,
                           MenuGamesMonitor& menu_games_monitor);

    void run() override;
    void stop() override;

    Queue<ServerMessage>& outgoing_queue();

    ~ClientHandler();
};

#endif


#include "client_handler.h"


#include <exception>
#include <utility>

ClientHandler::ClientHandler(Socket&& skt,
                             MenuGamesMonitor& menu_games_monitor):
        skt(std::move(skt)),
        outgoing(),
        menu_games_monitor(menu_games_monitor) 
{}


void ClientHandler::send_menu_result(const MenuResult& result, ActionType action, ServerProtocol& protocol) {
    ServerMessage message;

    if (result.success) {
        if (action == ActionType::CREATE_GAME) {
            message.type = MessageType::GAME_CREATED;
        } else {
            message.type = MessageType::GAME_JOINED;
        }
        message.game_code = result.game_code;
    } else {
        message.type = MessageType::ERROR;
        message.error = result.error;
    }

    protocol.send_message(message);
}

void ClientHandler::run() {
    ServerProtocol protocol(skt);

    try {
        while (should_keep_running()) {
            MenuRequest menu_request = protocol.recv_menu_request();

            MenuResult res{};
            res.success = false;
            res.error = "invalid menu request";

            switch (menu_request.type) {
                case ActionType::CREATE_GAME:
                    res = menu_games_monitor.create_game(menu_request.game_name, menu_request.player_name);
                    break;
                case ActionType::JOIN_GAME:
                    res = menu_games_monitor.join_game(menu_request.game_code, menu_request.player_name);
                    break;
                default:
                    break;
            }
            
            send_menu_result(res, menu_request.type, protocol);

            if (!res.success) {
                continue;
            }

            game = res.game;
            player_name = menu_request.player_name;


            break;
        }

    } catch (const std::exception& e) {
    }
}

void ClientHandler::stop() {
    if (stopped) {
        return;
    }

    stopped = true;

    receiver.stop();
    sender.stop();

    outgoing.close();

    try {
        skt.shutdown(2);
    } catch (const std::exception&) {
        // La conexión puede estar cerrada y si lanza una
        // excepcion podria no llegar a los joins de los threads,
        // por lo que se ignora la excepcion
    }
}

ClientHandler::~ClientHandler() {
    stop();
    receiver.join();
    sender.join();
}
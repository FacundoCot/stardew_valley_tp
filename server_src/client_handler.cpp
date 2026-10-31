
#include "client_handler.h"


#include <exception>
#include <utility>

ClientHandler::ClientHandler(Socket&& skt,
                             MenuGamesMonitor& menu_games_monitor,
                             int client_id,
                             ClientRegistry& registry):
        skt(std::move(skt)),
        outgoing(),
        client_id(client_id),
        menu_games_monitor(menu_games_monitor),
        registry(registry)
{}

int ClientHandler::id() const {
    return client_id;
}

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
                    res = menu_games_monitor.create_game(client_id, menu_request.game_name, menu_request.player_name);
                    break;
                case ActionType::JOIN_GAME:
                    res = menu_games_monitor.join_game(client_id, menu_request.game_code, menu_request.player_name);
                    break;
                default:
                    break;
            }

            if (res.success) {
                game = res.game;
                player_name = menu_request.player_name;
            }
            
            send_menu_result(res, menu_request.type, protocol);

            if (!res.success) {
                continue;
            }

            if (!should_keep_running()) {
                break;
            }

            receiver = std::make_unique<ClientReceiver>(skt, game->get_actions_queue(), client_id);
            sender = std::make_unique<ClientSender>(skt, outgoing);

            registry.add(client_id, &outgoing);

            receiver->start();
            reciever_started = true;
            sender->start();
            sender_started = true;

            receiver->join();
            reciever_started = false;

            break;
        }
    } catch (const std::exception& e) {
        // desconexion del cliente o error en la comunicacion
    }

    registry.remove(client_id);

    stop();

    if (reciever_started) {
        receiver->stop();
        receiver->join();
    }
    if (sender_started) {
        sender->stop();
        sender->join();
    }
}

void ClientHandler::stop() {
    // stop default de Thread de la catedra
    Thread::stop();
    
    try {
        outgoing.close();
    } catch (const std::exception&) {
        // La cola puede estar cerrada 
    }
    try {
        skt.shutdown(2);
    } catch (const std::exception&) {
        // La conexión puede estar cerrada y si lanza una
    }
}

Queue<ServerMessage>& ClientHandler::outgoing_queue() {
    return outgoing;
}

ClientHandler::~ClientHandler() {
    stop();
    join();
}

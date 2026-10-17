
#include "client_handler.h"


#include <exception>
#include <utility>

ClientHandler::ClientHandler(Socket&& skt,
                             Queue<std::shared_ptr<ClientAction>>& actions,
                             int client_id):
        skt(std::move(skt)),
        outgoing(),
        receiver(this->skt, actions, client_id),
        sender(this->skt, outgoing) {}

void ClientHandler::start() {
    receiver.start();
    sender.start();
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

bool ClientHandler::is_finished() {
    return (!receiver.is_alive()) || (!sender.is_alive());
}

ClientHandler::~ClientHandler() {
    stop();
    receiver.join();
    sender.join();
}
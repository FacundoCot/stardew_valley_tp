#include "acceptor.h"

#include <exception>
#include <utility>

Acceptor::Acceptor(Socket&& skt,
                   Queue<std::shared_ptr<ClientAction>>& actions,
                   ClientRegistry& registry):
    skt(std::move(skt)), actions(actions), registry(registry) {}

void Acceptor::stop() {
    Thread::stop();
    skt.shutdown(2);
    skt.close();
}

void Acceptor::clean() {
    for (auto& client: clients) {
        registry.remove(client->id());
        client->stop();
        // hago stop al handler y apago todos los socket por cada
        // client_handler vivo, asi desbloqueo posibles
        // recv 
    }

    // destruye todos los uniqueptr que hay en clients
    // y por lo tanto, se destruyen todos los clienthandlers
    // que quedan, y por lo tanto cada uno ejecuta el destructor
    // propio de clienthandler, el cual joinea los hilos
    clients.clear();
}

void Acceptor::reap() {
    auto it = clients.begin();

    while (it != clients.end()) {
        if ((*it)->is_finished()) { //desreferencio el iterador para acceder al unique_ptr y luego con -> accedo a clienthandler
            registry.remove((*it)->id());
            it = clients.erase(it);
        } else {
            ++it;
        }
    }
}

void Acceptor::run() {
    std::exception_ptr error = nullptr;

    try {
        while (should_keep_running()) {
            Socket new_client = skt.accept();

            if (!should_keep_running()) {
                break;
            }

            reap();

            // en vez de crear un client_handler comun creo uno que sea unique_ptr
            // asi es posible despues pushearlo al vector clients
            auto new_handler = std::make_unique<ClientHandler>(
                    std::move(new_client), actions, next_client_id++);
            
            registry.add(new_handler->id(), &new_handler->outgoing_queue());

            // le transfiero el owner al vector
            clients.push_back(std::move(new_handler));

            /*agarro el ultimo puntero de clients, que es el que pushee recien, y con
            -> accedo a lo que apunta ese puntero, que es un clienthandler, y ahi
            llamo a start() para que el hilo se inicie
            se hace lo de -> porque clients guarda punteros, no direc ClientHandler

            se lo pushea y despues ejecuta para que antes de que se este ejecutando
            ya se sepa quien es el dueño y el encargado del thread, no necesariamente
            rompe pero es mejor*/
            clients.back()->start();
        }
    } catch (...) {
        // si ocurrio una excepcion pero el hilo deberia haber seguido
        // corriendo entonces se guarda la excepcion y se tira despues
        // de limpiar el vector clients
        if (should_keep_running()) {
            error = std::current_exception();
        }

        // si no entra al if es que should_run dio false, entonces fue porque
        // se hizo un stop de afuera de la clase y el accept tiro una excepcion
        // pero es el cierre normal entonces no se avisa nada
    }

    clean();

    if (error) {  // si sigue siendo nullptr no entra
        std::rethrow_exception(error);
    }
}

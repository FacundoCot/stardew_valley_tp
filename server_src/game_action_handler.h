
#ifndef GAME_ACTION_HANDLER_H
#define GAME_ACTION_HANDLER_H

#include <string>

#include "../common_src/messages.h"

class GameActionHandler {
    public:
        // virtual sirve para permitir el polimorfismo, es decir, 
        // que podamos llamar a un método de una clase base y que se 
        // ejecute la implementación de la clase hija correspondiente.

        // = 0 indica que es un método puro virtual, es decir, 
        // que no tiene implementación en la clase base y que debe ser implementado en las clases hijas.
        virtual void move_player(int client_id,
                                Direction direction) = 0;

        virtual void disconnect_client(int client_id) = 0;

        // Destructor virtual: asegura que al destruir un objeto mediante un puntero
        // a la clase padre, se ejecuten también los destructores de las clases hijas
        virtual ~GameActionHandler() = default;
};

#endif

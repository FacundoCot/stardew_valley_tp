#ifndef GAME_LOOP_H
#define GAME_LOOP_H

#include <memory>

#include "../common_src/queue.h"
#include "../common_src/thread.h"

#include "client_action.h"
#include "game_manager.h"

// Hilo propio de una partida: saca las acciones de la cola y las aplica
// con el GameManager. Solo este hilo ejecuta acciones de la partida.
class GameLoop : public Thread {
private:
    GameManager& manager;
    Queue<std::shared_ptr<ClientAction>>& actions;

public:
    GameLoop(GameManager& manager, Queue<std::shared_ptr<ClientAction>>& actions);

    void run() override;
    void stop() override;

    ~GameLoop();
};

#endif
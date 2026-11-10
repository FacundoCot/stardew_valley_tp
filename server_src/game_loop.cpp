#include "game_loop.h"
#include <algorithm>
#include <exception>

#include "common_src/chronometer.h"
#include "common_src/define.h"

GameLoop::GameLoop(GameManager& manager_param,
                   Queue<std::shared_ptr<ClientAction>>& actions_param):
        manager(manager_param),
        actions(actions_param) {}


void GameLoop::run() {
    const float dt = static_cast<float>(RATE) / 1000.0f;
    Chronometer chronometer;

    try {
        while (should_keep_running()) {
            // 1) Procesa todas las acciones pendientes sin bloquear
            std::shared_ptr<ClientAction> action;
            while (actions.try_pop(action)) {
                action->execute(manager);
            }

            // 2) Espera al próximo tick y calcula cuántos ticks pasaron
            int ticks = chronometer.sleep_and_calculate_it();

            // 3) Avanza el mundo tantas veces como ticks corresponda
            for (int i = 0; i < ticks; i++) {
                manager.tick(dt);
            }
        }
    } catch (const ClosedQueue&) {
        // La cola se cerró: la partida termina
    }
}

void GameLoop::stop() {
    Thread::stop();
    
    try {
        actions.close();
    } catch (const std::exception&) {
        
    }
}

GameLoop::~GameLoop() {
    stop();
    join();
}
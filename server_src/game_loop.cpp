#include "game_loop.h"
#include "../common_src/chronometer.h"

#include <exception>

GameLoop::GameLoop(GameManager& manager_param,
                   Queue<std::shared_ptr<ClientAction>>& actions_param):
        manager(manager_param),
        actions(actions_param) {}

void GameLoop::run() {
    try {
        Chronometer chrono;
        while (should_keep_running()) {
            int iterations = chrono.sleep_and_calculate_it();

            for (int i = 0; i < iterations && should_keep_running(); i++) {
                std::shared_ptr<ClientAction> action;

                while (actions.try_pop(action)) {
                    action->execute(manager);
                }
            }
        }
    } catch (const ClosedQueue&) {
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
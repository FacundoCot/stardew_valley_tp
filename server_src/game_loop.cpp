#include "game_loop.h"

#include <exception>

GameLoop::GameLoop(GameManager& manager_param,
                   Queue<std::shared_ptr<ClientAction>>& actions_param):
        manager(manager_param),
        actions(actions_param) {}

void GameLoop::run() {
    try {
        while (should_keep_running()) {
            std::shared_ptr<ClientAction> action = actions.pop();
            action->execute(manager);
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
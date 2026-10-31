
#ifndef MESSAGES_H
#define MESSAGES_H

#include <cstdint>
#include <string>
#include <vector>
#include <memory>

enum class ActionType : uint8_t {
    CREATE_GAME = 1,
    JOIN_GAME = 2,
    MOVE = 3,
    DISCONNECT = 4
};

enum class Direction : uint8_t {
    UP = 1,
    DOWN = 2,
    LEFT = 3,
    RIGHT = 4
};

enum class MessageType : uint8_t {
    GAME_CREATED = 1,
    GAME_JOINED = 2,
    GAME_STATE = 3,
    ERROR = 4
};

struct PlayerState {
    std::string player_name;
    float x;
    float y;
};

class Game;

struct ServerMessage {
    MessageType type;
    std::string game_code;
    std::vector<PlayerState> players;
};

struct MenuResult {
    bool success;
    std::string error;
    std::string game_code;
    std::shared_ptr<Game> game;
};

struct MenuRequest {
    ActionType type;
    std::string game_name;
    std::string game_code;
    std::string player_name;
};

#endif
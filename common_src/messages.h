
#ifndef MESSAGES_H
#define MESSAGES_H

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <utility>
#include "item_types.h"

enum class ActionType : uint8_t {
  NULL_ACTION = 0,
  CREATE_GAME = 1,
  JOIN_GAME = 2,
  MOVE = 3,
  DISCONNECT = 4
};

enum class Direction : uint8_t {
  DEFAULT = 0,
  UP = 1,
  DOWN = 2,
  LEFT = 3,
  RIGHT = 4
};

enum class MessageType : uint8_t {
  GAME_CREATED = 1,
  GAME_JOINED = 2,
  GAME_STATE = 3,
  ERROR = 4,
  INVENTORY_STATE = 5
};

struct PlayerState {
    std::string player_name;
    float x;
    float y;
    int health;
    int max_health;
    bool unconscious;


    PlayerState(std::string name, float x, float y,
                int health, int max_health, bool unconscious):
        player_name(std::move(name)), x(x), y(y),
        health(health), max_health(max_health), unconscious(unconscious) {}
};

struct InventorySlotState {
    uint8_t index;
    ItemType type;
    Quality quality;
    uint8_t quantity;
};

struct InventoryState {
    uint32_t coins = 0;
    std::vector<InventorySlotState> slots;
};

struct ServerMessage {
  MessageType type;
  std::string game_code;
  std::vector<PlayerState> players;
  InventoryState inventory;
};

struct MenuRequest {
  ActionType type;
  std::string game_name;
  std::string game_code;
  std::string player_name;
};

struct ClientMessage {
  ActionType type;
  uint16_t game_name_len = 0;
  uint16_t player_name_len = 0;
  std::string player_name;
  Direction direction = Direction::DEFAULT;
  std::string game_name;
};

#endif

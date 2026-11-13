
#ifndef MESSAGES_H
#define MESSAGES_H

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <utility>
#include "character_appearance.h"
#include "item_types.h"

enum class DayFinished : uint8_t {
    NOT_FINISHED = 0,
    FINISHED = 1
};

enum class ActionType : uint8_t {
  NULL_ACTION = 0,
  CREATE_GAME = 1,
  JOIN_GAME = 2,
  MOVE = 3,
  DISCONNECT = 4,
  ATTACK = 5,
  CHAT = 6,
  LIST_GAMES = 7
};

enum class EnemyKind : uint8_t {
  SKELETON = 1,
  ORC = 2,
  VAMPIRE = 3,
  SKULL = 4
};

struct EnemyState {
  EnemyKind kind;
  float x;
  float y;
  int health;
  int max_health;
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
  INVENTORY_STATE = 5,
  TIME_STATE = 6,
  CHAT_MESSAGE = 7,
  GAMES_LIST = 8
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

    PlayerState() = default;
};

struct InventorySlotState {
    uint8_t index;
    ItemType type;
    Quality quality;
    uint8_t quantity;
};

struct InventoryState {
    uint32_t coins = 0;
    std::vector<InventorySlotState> slot_vector;
};

struct TimeState {
    uint16_t day;
    uint8_t hour;
    uint8_t minute;
    bool finished;
};

enum class ChatKind : uint8_t {
    PUBLIC = 0,
    PRIVATE = 1,
    NPC = 2,
    SYSTEM = 3
};

struct ChatState {
    ChatKind kind;
    std::string sender;
    std::string text;
};

struct GameInfoForMenu {
    std::string game_code;
    std::string game_name;
    std::string owner_name;
    bool multiplayer;
};

struct ServerMessage {
  MessageType type;
  std::string game_code;
  std::vector<PlayerState> players;
  std::vector<EnemyState> enemies;
  uint8_t return_code = 0;
  InventoryState inventory;
  TimeState time;
  ChatState chat;
  std::vector<GameInfoForMenu> games;
};

struct MenuRequest {
  ActionType type;
  std::string world_name = "por_defecto";
  std::string game_name;
  std::string game_code;
  std::string player_name;
  CharacterAppearance appearance;
};

struct ClientMessage {
  ActionType type;
  uint16_t game_name_len = 0;
  uint16_t player_name_len = 0;
  std::string player_name;
  Direction direction = Direction::DEFAULT;
  std::string game_name;
  CharacterAppearance appearance;
  std::string chat_text;
  std::string game_code; // momentáneo del join
};

#endif

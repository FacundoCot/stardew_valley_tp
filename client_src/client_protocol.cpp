#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>

#include "../common_src/messages.h"
#include "../common_src/socket.h"
#include "client_protocol.h"
#include <netinet/in.h>

#include <iostream>

ClientProtocol::ClientProtocol(Socket &&skt_rcv) : skt(std::move(skt_rcv)) {}

void ClientProtocol::append_u16_t(std::string &buffer, uint16_t appearance_item){
  uint16_t appendable = htons(appearance_item);
  buffer.append(reinterpret_cast<const char*>(&appendable), sizeof(uint16_t));
}

void ClientProtocol::serialize_appearance(const ClientMessage& msg_to_server, std::string& buffer){
  append_u16_t(buffer, msg_to_server.appearance.body);
  append_u16_t(buffer, msg_to_server.appearance.hair);
  append_u16_t(buffer, msg_to_server.appearance.shirt);
  append_u16_t(buffer, msg_to_server.appearance.pants);
  append_u16_t(buffer, msg_to_server.appearance.hat);
}

std::string
ClientProtocol::serialize_create_game(const ClientMessage &msg_to_server) {
  size_t len_msg = sizeof(uint8_t) + sizeof(uint16_t) + msg_to_server.game_name.length() +
                   sizeof(uint16_t) + msg_to_server.player_name.length();
  std::string buffer;
  buffer.reserve(len_msg);

  buffer += static_cast<uint8_t>(msg_to_server.type);

  uint16_t game_name_len_net = htons(msg_to_server.game_name_len);
  buffer.append(reinterpret_cast<const char *>(&game_name_len_net),
                sizeof(uint16_t));
  buffer += msg_to_server.game_name;

  uint16_t player_name_len_net = htons(msg_to_server.player_name_len);
  buffer.append(reinterpret_cast<const char *>(&player_name_len_net),
                sizeof(uint16_t));
  buffer += msg_to_server.player_name;

  serialize_appearance(msg_to_server, buffer);

  return buffer;
}

std::string
ClientProtocol::serialize_join_game(const ClientMessage &msg_to_server) {
  size_t len_msg = sizeof(uint8_t) + sizeof(uint16_t) +
      msg_to_server.player_name.length();
  std::string buffer;
  buffer.reserve(len_msg);

  buffer += static_cast<uint8_t>(msg_to_server.type);

  uint16_t code_len_net = htons(static_cast<uint16_t>(msg_to_server.game_code.size()));
  buffer.append(reinterpret_cast<const char *>(&code_len_net), sizeof(uint16_t));
  buffer += msg_to_server.game_code;

  uint16_t player_name_len_net = htons(msg_to_server.player_name_len);
  buffer.append(reinterpret_cast<const char *>(&player_name_len_net),
                sizeof(uint16_t));
  buffer += msg_to_server.player_name;

  serialize_appearance(msg_to_server, buffer);

  return buffer;
}

std::string ClientProtocol::serialize_move(const ClientMessage &msg_to_server) {
  size_t len_msg = sizeof(uint8_t) * 2;
  std::string buffer;
  buffer.reserve(len_msg);

  buffer += static_cast<uint8_t>(msg_to_server.type);
  buffer += static_cast<uint8_t>(msg_to_server.direction);

  return buffer;
}

std::string
ClientProtocol::serialize_disconnect(const ClientMessage &msg_to_server) {
  size_t len_msg = sizeof(uint8_t);
  std::string buffer;
  buffer.reserve(len_msg);

  buffer += static_cast<uint8_t>(msg_to_server.type);

  return buffer;
}

std::string ClientProtocol::serialize(const ClientMessage &msg_to_server) {
  switch (msg_to_server.type) {
  case ActionType::CREATE_GAME:
    return serialize_create_game(msg_to_server);

  case ActionType::JOIN_GAME:
    return serialize_join_game(msg_to_server);

  case ActionType::MOVE:
    return serialize_move(msg_to_server);

  case ActionType::DISCONNECT:
    return serialize_disconnect(msg_to_server);

  default:
    throw std::runtime_error("Error at: client_protocol: action not exists!");
  }
}

void ClientProtocol::send_to_server(const ClientMessage &msg_to_server) {
  std::string buf = serialize(msg_to_server);

  if (skt.sendall(buf.data(), buf.size()) != static_cast<int>(buf.size())) {
    throw std::runtime_error("Error at: client_protocol: Socket was closed!");
  }
}

ServerMessage ClientProtocol::rcv_from_server() {
  ServerMessage response{};
  skt.recvall(&response.type, sizeof(uint8_t));

  switch (response.type) {
  case MessageType::GAME_CREATED:
    response.game_code = rcv_string();
    break;
  case MessageType::GAME_JOINED:
    response.game_code = rcv_string();
    break;
  case MessageType::GAME_STATE: {
    uint16_t count;
    skt.recvall(&count, sizeof(uint16_t));
    count = ntohs(count);

    for (uint16_t i = 0; i < count; i++) {
      PlayerState player;
      player.player_name = rcv_string();

      uint32_t coordinate;
      skt.recvall(&coordinate, sizeof(uint32_t));
      float x = ntohl(coordinate) / 100.0f;
      skt.recvall(&coordinate, sizeof(uint32_t));
      float y = ntohl(coordinate) / 100.0f;

      player.x = x;
      player.y = y;

      uint16_t health, max_health;
      skt.recvall(&health, sizeof(uint16_t));
      skt.recvall(&max_health, sizeof(uint16_t));
      player.health = ntohs(health);
      player.max_health = ntohs(max_health);

      uint16_t unconscious;
      skt.recvall(&unconscious, sizeof(uint8_t));
      player.unconscious = unconscious;

      response.players.push_back(player);
    }


    uint16_t enemy_count;
    skt.recvall(&enemy_count, sizeof(uint16_t));
    enemy_count = ntohs(enemy_count);

    for (uint16_t i = 0; i < enemy_count; i++) {
      EnemyState enemy;
      
      uint8_t kind;
      skt.recvall(&kind, sizeof(uint8_t));
      enemy.kind = static_cast<EnemyKind>(kind);

      uint32_t coordinate;
      skt.recvall(&coordinate, sizeof(uint32_t));
      enemy.x = ntohl(coordinate) / 100.0f;
      
      skt.recvall(&coordinate, sizeof(uint32_t));
      enemy.y = ntohl(coordinate) / 100.0f;

      uint16_t health;
      skt.recvall(&health, sizeof(uint16_t));
      enemy.health = ntohs(health);

      uint16_t max_health;
      skt.recvall(&max_health, sizeof(uint16_t));
      enemy.max_health = ntohs(max_health);

      response.enemies.push_back(enemy);
    }

    break;
  }
  case MessageType::INVENTORY_STATE: {
    uint32_t coins;
    skt.recvall(&coins, sizeof(uint32_t));
    response.inventory.coins = ntohl(coins);

    uint8_t count;
    skt.recvall(&count, sizeof(uint8_t));

    for (uint8_t i = 0; i < count; i++) {
      InventorySlotState slot;

      skt.recvall(&slot.index, sizeof(uint8_t));

      uint8_t type_raw, quality_raw;
      skt.recvall(&type_raw, sizeof(uint8_t));
      skt.recvall(&quality_raw, sizeof(uint8_t));

      slot.type = static_cast<ItemType>(type_raw);
      slot.quality = static_cast<Quality>(quality_raw);

      skt.recvall(&slot.quantity, sizeof(uint8_t));

      response.inventory.slot_vector.push_back(slot);
    }
    break;
  }
  case MessageType::TIME_STATE: {
    uint16_t day_network;
    skt.recvall(&day_network, sizeof(uint16_t));
    response.time.day = ntohs(day_network);

    skt.recvall(&response.time.hour, sizeof(uint8_t));
    skt.recvall(&response.time.minute, sizeof(uint8_t));
    skt.recvall(&response.time.finished, sizeof(uint8_t));
    break;
  }

  case MessageType::ERROR:
    break;
  default:
    throw std::runtime_error("Invalid server message.");
  }

  return response;
}

std::string ClientProtocol::rcv_string() {
  uint16_t length;

  skt.recvall(&length, sizeof(uint16_t));
  length = ntohs(length);

  if (length > 1024) {
    throw std::runtime_error("String too long.");
  }

  std::string value(length, '\0');
  skt.recvall(value.data(), length);

  return value;
}

void ClientProtocol::shutdown() {
  skt.shutdown(2);
  skt.close();
}

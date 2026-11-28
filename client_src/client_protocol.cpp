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
  append_u16_t(buffer, msg_to_server.appearance.hair_color);
  append_u16_t(buffer, msg_to_server.appearance.pants_color);
}
    

std::string ClientProtocol::serialize_create_game(const ClientMessage& msg_to_server) {
    std::string buffer;

    buffer += static_cast<char>(ActionType::CREATE_GAME);

    append_u16_t(buffer, static_cast<uint16_t>(msg_to_server.game_name.size()));
    buffer += msg_to_server.game_name;

    serialize_appearance(msg_to_server, buffer);

    return buffer;
}

std::string ClientProtocol::serialize_join_game(const ClientMessage& msg_to_server) {

    std::string buffer;

    buffer += static_cast<char>(ActionType::JOIN_GAME);

    append_u16_t(buffer, static_cast<uint16_t>(msg_to_server.game_code.size()));
    buffer += msg_to_server.game_code;

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

std::string ClientProtocol::serialize_confirm_day(const ClientMessage &msg_to_server) {
  size_t len_msg = sizeof(uint8_t);
  std::string buffer;
  buffer.reserve(len_msg);

  buffer += static_cast<uint8_t>(msg_to_server.type);

  return buffer;
}

std::string ClientProtocol::serialize_select_slot(const ClientMessage &msg_to_server) {
  size_t len_msg = sizeof(uint8_t)*2;
  std::string buffer;
  buffer.reserve(len_msg);

  buffer += static_cast<uint8_t>(msg_to_server.type);
  buffer += msg_to_server.slot_index;

  return buffer;
}

std::string ClientProtocol::serialize_chat(const ClientMessage &msg_to_server) {
  size_t len_msg = msg_to_server.chat_text.length() + sizeof(uint16_t) + sizeof(uint8_t);
  std::string buffer;
  buffer.reserve(len_msg);

  buffer += static_cast<uint8_t>(msg_to_server.type);
  
  append_u16_t(buffer, static_cast<uint16_t>(msg_to_server.chat_text.length()));
  buffer += msg_to_server.chat_text;

  return buffer;
}

std::string ClientProtocol::serialize_username(const ClientMessage &msg_to_server) {
  size_t len_msg = sizeof(uint8_t) + sizeof(uint16_t) + msg_to_server.player_name.size();
  std::string buffer;
  buffer.reserve(len_msg);

  buffer += static_cast<uint8_t>(msg_to_server.type);

  uint16_t name_len_net = htons(static_cast<uint16_t>(msg_to_server.player_name.size()));
  buffer.append(reinterpret_cast<const char *>(&name_len_net), sizeof(uint16_t));
  buffer += msg_to_server.player_name;

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
  
  case ActionType::CONFIRM_NEXT_DAY:
    return serialize_confirm_day(msg_to_server);

  case ActionType::SELECT_SLOT:
    return serialize_select_slot(msg_to_server);

  case ActionType::CHAT:
    return serialize_chat(msg_to_server);

  case ActionType::USERNAME:
    return serialize_username(msg_to_server);

  case ActionType::LIST_GAMES:
    return std::string(1, static_cast<char>(ActionType::LIST_GAMES));

  case ActionType::ENABLE_MULTIPLAYER:
    return std::string(1, static_cast<char>(ActionType::ENABLE_MULTIPLAYER));
  
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

void ClientProtocol::recv_or_throw(void *data, unsigned int size) {
  if (size > 0 && skt.recvall(data, size) == 0) {
    throw std::runtime_error("Socket closed.");
  }
}

ServerMessage ClientProtocol::rcv_from_server() {
  ServerMessage response{};
  recv_or_throw(&response.type, sizeof(uint8_t));

  switch (response.type) {
  case MessageType::GAME_CREATED:
    response.game_code = rcv_string();
    break;
  case MessageType::GAME_JOINED:
    response.game_code = rcv_string();
    break;

  case MessageType::MULTIPLAYER_ENABLED:
    response.game_code = rcv_string();
    break;

  case MessageType::GAME_STATE: {


    response.zone = rcv_zone_id();
    response.self_player = rcv_player_state();

    uint16_t count_player;
    recv_or_throw(&count_player, sizeof(uint16_t));
    count_player = ntohs(count_player);

    response.players.reserve(count_player);

    for(uint16_t i = 0; i < count_player; i++) {
      response.players.push_back(rcv_player_state());
    }

    uint16_t map_count;
    recv_or_throw(&map_count, sizeof(uint16_t));
    map_count = ntohs(map_count);

    response.map_players.reserve(map_count);

    for (uint16_t i = 0; i < map_count; i++) {
      MapPlayerState player;

      player.player_name = rcv_string();

      uint32_t coordinate;

      recv_or_throw(&coordinate, sizeof(uint32_t));
      player.x = ntohl(coordinate) / 100.0f;

      recv_or_throw(&coordinate, sizeof(uint32_t));
      player.y = ntohl(coordinate) / 100.0f;

      player.zone = rcv_zone_id();
      response.map_players.push_back(player);
    }

    uint16_t enemy_count;
    recv_or_throw(&enemy_count, sizeof(uint16_t));
    enemy_count = ntohs(enemy_count);

    for (uint16_t i = 0; i < enemy_count; i++) {
      EnemyState enemy;
      
      uint8_t kind;
      recv_or_throw(&kind, sizeof(uint8_t));
      enemy.kind = static_cast<EnemyKind>(kind);

      uint32_t coordinate;
      recv_or_throw(&coordinate, sizeof(uint32_t));
      enemy.x = ntohl(coordinate) / 100.0f;
      
      recv_or_throw(&coordinate, sizeof(uint32_t));
      enemy.y = ntohl(coordinate) / 100.0f;

      uint16_t health;
      recv_or_throw(&health, sizeof(uint16_t));
      enemy.health = ntohs(health);

      uint16_t max_health;
      recv_or_throw(&max_health, sizeof(uint16_t));
      enemy.max_health = ntohs(max_health);

      uint8_t variant, frenzy;

      recv_or_throw(&variant, sizeof(uint8_t));
      recv_or_throw(&frenzy, sizeof(uint8_t));

      enemy.variant = static_cast<EnemyVariant>(variant);
      enemy.frenzy = frenzy != 0;

      response.enemies.push_back(enemy);
    }
    rcv_projectiles(response);
    rcv_inventory(response);
    rcv_time_state(response);
    rcv_plot(response);

    uint8_t multiplayer_enabled;
    recv_or_throw(&multiplayer_enabled, sizeof(uint8_t));
    response.multiplayer_enabled = multiplayer_enabled != 0;

    break;
  }
  case MessageType::GAMES_LIST: {
    uint16_t count;
    recv_or_throw(&count, sizeof(count));
    count = ntohs(count);

    for (uint16_t i = 0; i < count; i++) {
        GameInfoForMenu game{};

        game.game_code = rcv_string();
        game.game_name = rcv_string();
        game.owner_name = rcv_string();

        uint8_t multiplayer;
        recv_or_throw(&multiplayer, sizeof(multiplayer));
        game.multiplayer = multiplayer != 0;

        game.player_name = rcv_string();
        game.appearance = rcv_appearance();

        response.games.push_back(game);
    }
    break;
  }
  case MessageType::WORLD_INFO:
    rcv_world_info(response);
    break;

  case MessageType::CHAT_MESSAGE:
    rcv_chat_message(response);
    break;
  case MessageType::USERNAME:
    break;
  case MessageType::ERROR:
    break;
  default:
    throw std::runtime_error("Invalid server message.");
  }

  return response;
}

ZoneId ClientProtocol::rcv_zone_id() {
    ZoneId zone;
    uint8_t kind, lvl;
    recv_or_throw(&kind, sizeof(uint8_t));
    recv_or_throw(&lvl, sizeof(uint8_t));

    zone.kind = static_cast<ZoneId::Kind>(kind);
    zone.level = lvl;

    return zone;
}

std::string ClientProtocol::rcv_string() {
  uint16_t length;

  recv_or_throw(&length, sizeof(uint16_t));
  length = ntohs(length);

  if (length > 1024) {
    throw std::runtime_error("String too long.");
  }

  std::string value(length, '\0');
  recv_or_throw(value.data(), length);

  return value;
}

void ClientProtocol::shutdown() {
  skt.shutdown(2);
  skt.close();
}

CharacterAppearance ClientProtocol::rcv_appearance(){
  uint16_t parts[7] = {};
  for(uint16_t& p : parts){
    recv_or_throw(&p, sizeof(uint16_t));
    p = ntohs(p);
  }
  
  return CharacterAppearance{parts[0], parts[1], parts[2], parts[3], parts[4], parts[5], parts[6]};
}

void ClientProtocol::rcv_inventory(ServerMessage &response) {
  uint32_t coins;
  recv_or_throw(&coins, sizeof(uint32_t));
  response.inventory.coins = ntohl(coins);

  uint8_t selected_slot;
  recv_or_throw(&selected_slot, sizeof(uint8_t));
  response.inventory.selected_slot = selected_slot;

  uint8_t count;
  recv_or_throw(&count, sizeof(uint8_t));

  for (uint8_t i = 0; i < count; i++) {
    InventorySlotState slot;

    recv_or_throw(&slot.index, sizeof(uint8_t));

    uint8_t type_raw, quality_raw;
    recv_or_throw(&type_raw, sizeof(uint8_t));
    recv_or_throw(&quality_raw, sizeof(uint8_t));

    slot.type = static_cast<ItemType>(type_raw);
    slot.quality = static_cast<Quality>(quality_raw);

    recv_or_throw(&slot.quantity, sizeof(uint8_t));

    response.inventory.slot_vector.push_back(slot);
  }
}

void ClientProtocol::rcv_time_state(ServerMessage &response) {
  uint16_t day_network;
  recv_or_throw(&day_network, sizeof(uint16_t));
  response.time.day = ntohs(day_network);

  recv_or_throw(&response.time.hour, sizeof(uint8_t));
  recv_or_throw(&response.time.minute, sizeof(uint8_t));
  recv_or_throw(&response.time.finished, sizeof(uint8_t));
}

void ClientProtocol::rcv_world_info(ServerMessage &response) {
  uint32_t size_net;
  recv_or_throw(&size_net, sizeof(uint32_t));

  uint32_t size = ntohl(size_net);

  std::vector<uint8_t> world_info_net(size);

  recv_or_throw(world_info_net.data(), size);

  response.world_info = std::move(world_info_net);

}

PlayerState ClientProtocol::rcv_player_state() {
    PlayerState player;
    player.player_name = rcv_string();

    uint32_t coordinate;
    recv_or_throw(&coordinate, sizeof(uint32_t));
    float x = ntohl(coordinate) / 100.0f;
    recv_or_throw(&coordinate, sizeof(uint32_t));
    float y = ntohl(coordinate) / 100.0f;

    player.x = x;
    player.y = y;

    uint16_t health, max_health;
    recv_or_throw(&health, sizeof(uint16_t));
    recv_or_throw(&max_health, sizeof(uint16_t));
    player.health = ntohs(health);
    player.max_health = ntohs(max_health);

    uint16_t energy, max_energy;
    recv_or_throw(&energy, sizeof(uint16_t));
    recv_or_throw(&max_energy, sizeof(uint16_t));

    player.energy = htons(energy);
    player.max_energy = htons(max_energy);

    uint8_t unconscious;
    recv_or_throw(&unconscious, sizeof(uint8_t));
    player.unconscious = unconscious;

    player.appearance = rcv_appearance();

    uint8_t facing;
    recv_or_throw(&facing, sizeof(uint8_t));
    player.facing = static_cast<Direction>(facing);

    return player;
  
}

void ClientProtocol::rcv_projectiles(ServerMessage &response) {
    uint16_t count_net;
    recv_or_throw(&count_net, sizeof(uint16_t));

    uint16_t count = ntohs(count_net);

    response.projectiles.reserve(count);

    for (uint16_t i = 0; i < count; i++) {
      ProjectileState projectil;

      uint8_t variant; 
      recv_or_throw(&variant, sizeof(uint8_t));
      projectil.variant = static_cast<EnemyVariant>(variant);

      uint32_t coordinate;

      recv_or_throw(&coordinate, sizeof(uint32_t));
      projectil.x = ntohl(coordinate);

      recv_or_throw(&coordinate, sizeof(uint32_t));
      projectil.y = coordinate;

      response.projectiles.push_back(projectil);

    }

}

void ClientProtocol::rcv_plot(ServerMessage& response) {
  uint16_t plot_count_net;

  recv_or_throw(&plot_count_net, sizeof(uint16_t));

  uint16_t plot_count = ntohs(plot_count_net);
  
  response.farm_plots.reserve(plot_count);

  for (uint16_t i = 0; i < plot_count; i++) {
    FarmPlotState plot;

    uint16_t x, y, grown, required;
    uint8_t has_crop, crop_type, needs_water, ready;

    recv_or_throw(&x, sizeof(uint16_t));
    recv_or_throw(&y, sizeof(uint16_t));
    recv_or_throw(&grown, sizeof(uint16_t));
    recv_or_throw(&required, sizeof(uint16_t));

    recv_or_throw(&has_crop, sizeof(uint8_t));
    recv_or_throw(&crop_type, sizeof(uint8_t));
    recv_or_throw(&needs_water, sizeof(uint8_t));
    recv_or_throw(&ready, sizeof(uint8_t));

    plot.cell_x = ntohs(x);
    plot.cell_y = ntohs(y);
    plot.days_grown = htons(grown);
    plot.growth_days = htons(required);

    plot.has_crop = has_crop;
    plot.crop_type = static_cast<ItemType>(crop_type);
    plot.needs_water = needs_water != 0;
    plot.ready_to_harvest = ready != 0;

    response.farm_plots.push_back(plot);
  }
}

void ClientProtocol::rcv_chat_message(ServerMessage &response) {
  uint8_t kind;
  recv_or_throw(&kind, sizeof(uint8_t));

  response.chat.kind = static_cast<ChatKind>(kind);
  response.chat.sender = rcv_string();
  response.chat.text = rcv_string();
}
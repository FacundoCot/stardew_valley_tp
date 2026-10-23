#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>

#include "../common_src/messages.h"
#include "../common_src/socket.h"
#include "client_protocol.h"
#include <netinet/in.h>

ClientProtocol::ClientProtocol(Socket &&skt_rcv) : skt(std::move(skt_rcv)) {}

std::string
ClientProtocol::serialize_create_game(const ClientMessage &msg_to_server) {
  size_t len_msg = sizeof(uint8_t) + sizeof(msg_to_server.game_name) +
                   sizeof(msg_to_server.player_name);
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

  return buffer;
}

std::string
ClientProtocol::serialize_join_game(const ClientMessage &msg_to_server) {
  size_t len_msg =
      sizeof(msg_to_server.game_name) + sizeof(msg_to_server.player_name);
  std::string buffer;
  buffer.reserve(len_msg);

  buffer += static_cast<uint8_t>(msg_to_server.type);

  uint16_t game_name_len_net = htons(msg_to_server.game_name_len);
  buffer.append(reinterpret_cast<const char *>(&game_name_len_net),
                sizeof(uint16_t));
  buffer += msg_to_server.game_name;

  uint16_t player_name_len_net = htons(msg_to_server.game_name_len);
  buffer.append(reinterpret_cast<const char *>(&player_name_len_net),
                sizeof(uint16_t));
  buffer += msg_to_server.player_name;

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
ClientProtocol::serialize_disconnect(const ClientMessage &mst_to_server) {
  return "";
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
    throw std::runtime_error("Error at: client_protocol: action not exists!\n");
  }
}

void ClientProtocol::send_to_server(const ClientMessage &msg_to_server) {
  std::string buf = serialize(msg_to_server);

  if (skt.sendall(buf.data(), buf.size()) != static_cast<int>(buf.size())) {
    throw std::runtime_error("Error at: client_protocol: Socket was closed!\n");
  }
}

void ClientProtocol::shutdown() {
  skt.shutdown(2);
  skt.close();
}

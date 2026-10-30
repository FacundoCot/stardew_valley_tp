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
    throw std::runtime_error("Error at: client_protocol: action not exists!\n");
  }
}

void ClientProtocol::send_to_server(const ClientMessage &msg_to_server) {
  std::string buf = serialize(msg_to_server);

  if (skt.sendall(buf.data(), buf.size()) != static_cast<int>(buf.size())) {
    throw std::runtime_error("Error at: client_protocol: Socket was closed!\n");
  }
}

ServerMessage ClientProtocol::rcv_from_server() { 
  ServerMessage response{};
  skt.recvall(&response.type, sizeof(uint8_t));
  
  switch(response.type){
    case MessageType::GAME_CREATED:
      break;
    case MessageType::GAME_JOINED:
      response.game_code = rcv_string();
      break;
    case MessageType::GAME_STATE:{
      uint16_t count;
      skt.recvall(&count, sizeof(uint16_t));
      count = ntohs(count);
      
      for(uint16_t i = 0; i < count; i++){
        PlayerState player;
        player.player_name = rcv_string();
        
        uint32_t coordinate;
        skt.recvall(&coordinate, sizeof(uint32_t));
        float x = ntohl(coordinate)/100.0f;
        skt.recvall(&coordinate, sizeof(uint32_t));
        float y = ntohl(coordinate)/100.0f;

        player.x = x;
        player.y = y;

        response.players.push_back(player);
      }
      
      break;
    }
      
    case MessageType::ERROR:
      response.error = rcv_string();
      break;
    default:
      throw std::runtime_error("Invalid server message.");
  }

  return response; 
}

std::string ClientProtocol::rcv_string(){
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

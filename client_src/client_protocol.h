#ifndef CLIENT_PROTOCOL_H
#define CLIENT_PROTOCOL_H

#include <string>

#include "../common_src/messages.h"
#include "../common_src/socket.h"

class ClientProtocol {
private:
  Socket skt;

  std::string serialize(const ClientMessage &msg_to_server);
  std::string serialize_create_game(const ClientMessage &msg_to_server);
  std::string serialize_join_game(const ClientMessage &msg_to_server);
  std::string serialize_move(const ClientMessage &msg_to_server);
  std::string serialize_disconnect(const ClientMessage &msg_to_server);
  std::string serialize_confirm_day(const ClientMessage &msg_to_server);
  std::string serialize_select_slot(const ClientMessage &msg_to_server);
  std::string serialize_username(const ClientMessage &msg_to_server);

  void append_u16_t(std::string& buffer, uint16_t appearance_item);
  void serialize_appearance(const ClientMessage& msg_to_server, std::string& buffer);
  std::string rcv_string();
  CharacterAppearance rcv_appearance();
public:
  explicit ClientProtocol(Socket &&skt_rcv);

  void send_to_server(const ClientMessage &msg_to_server);
  ServerMessage rcv_from_server();
  void rcv_inventory(ServerMessage &msg);
  void rcv_time_state(ServerMessage &msg);
  void rcv_world_info(ServerMessage &response);
  void shutdown();
};

#endif

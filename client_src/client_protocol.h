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

public:
  explicit ClientProtocol(Socket &&skt_rcv);

  void send_to_server(const ClientMessage &msg_to_server);
  ServerMessage rcv_from_server();
  void shutdown();
};

#endif

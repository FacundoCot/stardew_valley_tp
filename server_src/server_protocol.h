#ifndef SERVER_PROTOCOL_H
#define SERVER_PROTOCOL_H

#include <memory>
#include <string>

#include "../common_src/messages.h"
#include "../common_src/socket.h"
#include "../common_src/world.h"

#include "client_action.h"

class ServerProtocol {
private:
  Socket &skt;

  void recv_or_throw(void *data, unsigned int size);
  void send_or_throw(const void *data, unsigned int size);

  std::string recv_string();
  void send_string(const std::string &str);

  CharacterAppearance recv_appearance();
  void send_appearance(const CharacterAppearance& appearance);
  void send_appearance_part(const uint16_t part);

  void send_inventory_state(const InventoryState& inventory);
  void send_time_state(const TimeState& time);
  void send_chat_message(const ChatState& chat);
  void send_games_list(const std::vector<GameInfoForMenu>& games);
  void send_zone_id(ZoneId zone);
  void send_player_state(const PlayerState& player);
  void send_farm_plots(const std::vector<FarmPlotState>& plots);
public:
  explicit ServerProtocol(Socket &skt);

    // se usa shared pointer porque la queue hace copias de los punteros
    // entonces no podemos usar unique_ptr, y no queremos usar raw pointer porque
    // no queremos preocuparnos por liberar la memoria de los punteros que
    // se guardan en la queue, entonces usamos shared_ptr para que se libere
    // automáticamente cuando no haya más referencias a ese puntero
    std::shared_ptr<ClientAction> recv_action();
    void send_message(const ServerMessage& message);
    void send_world_info(const World& world);
    MenuRequest recv_menu_request();
};

#endif

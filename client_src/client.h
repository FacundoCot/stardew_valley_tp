#ifndef CLIENT_H
#define CLIENT_H

#include "../common_src/chronometer.h"
#include "../common_src/messages.h"
#include "../common_src/queue.h"
#include "camera.h"
#include "client_protocol.h"
#include "eventhandler.h"
#include "player_visual.h"
#include "receiver.h"
#include "sender.h"
#include "visual.h"
#include "clock.h"
#include "button.h"
#include "inventory.h"
#include "health_energy.h"
#include "map_visual.h"
#include "menu_in_game.h"

#include <vector>
#include <SDL2pp/Rect.hh>


class Client {

private:
  

  Queue<ServerMessage>& responses;
  Queue<ClientMessage> commands;

  ClientProtocol& protocol;
  Sender sender;

  std::vector<PlayerState> players;
  PlayerState self;
  Camera camera;
  Visual visual; 
  Loader loader;
  PlayerVisual p_visual;
  HealthEnergy h_and_e;
  Clock clock;
  Inventory inventory;
  MenuInGame menu_game;
  EventHandler e_handler;

  ZoneId zone_player = ZoneId::FARM_ZONE;

  ClientState current_state = ClientState::PLAYING;

  uint8_t r = 255;
  uint8_t g = 255;
  uint8_t b = 255;

  bool running = true;
  bool was_intentional = false;

  void stop();

public:
  Client(Queue<ServerMessage>& responses, ClientProtocol &protocol);


  void run();
  void simulate_it(Button& btn, int& it, MapVisual& m_visual);

  void render(Button& btn, int& it, MapVisual& m_visual);

  void update(ServerMessage& response);

  ~Client();
};

#endif

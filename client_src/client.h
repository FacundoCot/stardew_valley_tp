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

#include <vector>
#include <SDL2pp/Rect.hh>


class Client {

private:
  EventHandler e_handler;

  Queue<ServerMessage> responses;
  Queue<ClientMessage> commands;

  ClientProtocol protocol;
  Receiver receiver;
  Sender sender;

  std::vector<PlayerState> players;
  Camera camera;

  ClientState current_state = ClientState::PLAYING;

  uint8_t r = 255;
  uint8_t g = 255;
  uint8_t b = 255;

  bool running = true;
  bool was_intentional = false;

  bool main_menu(int argc, char *argv[]);

public:
  Client(EventHandler &&e_handler_rcv, ClientProtocol &&protocol);


  void run(int argc, char* argv[]);
  void simulate_it(Clock& clock, Visual& visual, Button& btn, PlayerVisual& p_visual, Inventory& inventory, Loader& loader);

  void stop();
  void render(Clock& clock, Visual& visual, Button& btn, PlayerVisual& p_visual, Inventory& Inventory, Loader& loader);
  void update(ServerMessage& response, Clock& clock, Inventory& inventory);

  ~Client();
};

#endif

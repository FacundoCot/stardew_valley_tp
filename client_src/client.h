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

#include <vector>

class Client {

private:
  EventHandler e_handler;

  Queue<ServerMessage> responses;
  Queue<ClientMessage> commands;

  ClientProtocol protocol;
  Receiver receiver;
  Sender sender;

  std::vector<PlayerState> players;

  bool running = true;
  bool was_intentional = false;

  bool main_menu(int argc, char *argv[]);

public:
  Client(EventHandler &&e_handler_rcv, ClientProtocol &&protocol);


  void run(int argc, char* argv[]);
  void simulate_it(Visual &visual, Camera &camera, PlayerVisual &p_visual);

  void stop();

  ~Client();
};

#endif

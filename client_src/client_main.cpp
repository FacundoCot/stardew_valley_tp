#include <iostream>

#include "../common_src/define.h"
#include "../common_src/socket.h"
#include "../common_src/queue.h"
#include "../common_src/messages.h"
#include "receiver.h"
#include "game/menu_in_game.h"

#include "menu/menu.h"
#include "game/client.h"
#include "client_protocol.h"
#include "game/eventhandler.h"
#include "menu/ui_mainwindow.h"

int main(int argc, char *argv[]) {
  if (argc != CLIENT_PARAMETERS) {
    std::cerr << "Bad program call, expected: ./program <host> <port>"
              << "\n";
    return 1;
  }

  try {
    const char *host = argv[1];
    const char *port = argv[2];

    ClientProtocol protocol = ClientProtocol(std::move(Socket(host, port)));
    Queue<ServerMessage> responses;
    Receiver receiver(responses, protocol);
    receiver.start();
    
    bool has_exited;
    {
      Menu menu = Menu(protocol, responses);
      has_exited = menu.run(argc, argv);
      
    }

    if(!has_exited){
      Client client = Client(responses, protocol);
      client.run();
    }

    receiver.stop();

    try {
      protocol.shutdown();
    } catch (std::exception &) {
    }

    responses.close();
    receiver.join();

    return 0;

  } catch (const std::exception &e) {
    std::cerr << e.what() << "\n";
    return 1;
  } catch (...){
    std::cerr << "Error desconocido." << "\n";
    return 1;
  }

}

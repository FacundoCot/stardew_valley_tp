#include <iostream>

#include "../common_src/define.h"
#include "../common_src/socket.h"

#include "menu.h"
#include "client.h"
#include "client_protocol.h"
#include "eventhandler.h"
#include "ui_mainwindow.h"

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
    
    Menu menu = Menu(protocol);
    bool has_exited = menu.run(argc, argv);
    
    if(!has_exited){
      EventHandler handler;
      Client client = Client(std::move(handler), std::move(protocol));
      client.run();
    }
  } catch (const std::exception &e) {
    std::cerr << e.what() << "\n";
    return 1;
  } catch (...){
    std::cerr << "Error desconocido." << "\n";
    return 1;
  }

  return 0;
}

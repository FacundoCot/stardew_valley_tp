#include <iostream>
#include <utility>

#include "common_src/socket.h"
#include "server_src/server.h"

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Uso: " << argv[0] << " <servername>\n";
    return 1;
  }
  try {
    const char *servername = argv[1];

    Server srv = Server(Socket(servername));

    srv.run();
    return 0;

  } catch (const std::exception &err) {
    std::cerr << err.what() << "\n";
    return 1;
  }
}

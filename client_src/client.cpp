#include "client.h"
#include "../common_src/messages.h"
#include "eventhandler.h"

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>
#include <utility>

Client::Client(EventHandler &&e_handler_rcv, ClientProtocol &&protocol_rcv)
    : e_handler(std::move(e_handler_rcv)), protocol(std::move(protocol_rcv)) {}

void Client::run() {

  while (true) {
    // lógica de chronometer para calcular frames

    SDL_Event e;
    while (SDL_PollEvent(&e)) {

      ClientMessage msg = e_handler.get_next_event(e);
      // todo: tener un hilo sender
      protocol.send_to_server(msg);
    }

    // Hilo receiver se encarga de traer las respuestas del server.
    // Luego se renderiza con sdl2pp.

    // chronometer pone a dormir.
  }
}

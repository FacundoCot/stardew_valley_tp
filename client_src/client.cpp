#include "client.h"
#include "../common_src/messages.h"
#include "../common_src/queue.h"
#include "eventhandler.h"

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>
#include <utility>

Client::Client(EventHandler &&e_handler_rcv, ClientProtocol &&protocol_rcv)
    : e_handler(std::move(e_handler_rcv)), protocol(std::move(protocol_rcv)),
      receiver(responses, protocol), sender(protocol, commands) {}

void Client::run() {

  receiver.start();
  sender.start();
  bool was_intentional = false;

  try {
    while (running) {
      // lógica de chronometer para calcular frames

      SDL_Event event;
      while (SDL_PollEvent(&event)) {
        if (e_handler.is_disconnect(event)) {
          was_intentional = true;
        }

        ClientMessage msg = e_handler.get_next_event(event);

        commands.push(msg);
      }

      // Hilo receiver se encarga de traer las respuestas del server.
      ServerMessage response;
      while (responses.try_pop(response)) {
        // acá se actualiza el estado
      }

      // se renderiza con sdl2pp.
      // chronometer pone a dormir.
    }

  } catch (const ClosedQueue &e) {
    if (was_intentional) {
      // se cierra ventana y sale en silencio
    }
    std::cerr << e.what() << "\n";

  } catch (...) {
    std::cerr << "Unknown error at client!"
              << "\n";
  }
}

void Client::stop() {
  if (!running) {
    return;
  }
  running = false;

  receiver.stop();
  sender.stop();

  try {
    protocol.shutdown();

  } catch (const std::exception &) {
  }
}

Client::~Client() {
  stop();
  receiver.join();
  sender.join();
}

#include "client.h"
#include "../common_src/chronometer.h"
#include "../common_src/custom_error.h"
#include "../common_src/define.h"
#include "../common_src/messages.h"
#include "../common_src/queue.h"
#include "camera.h"
#include "eventhandler.h"
#include "loader.h"
#include "mainwindow.h"
#include "map_visual.h"
#include "player_visual.h"
#include "visual.h"
#include "../common_src/character_appearance.h"

#include <QApplication>
#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>
#include <utility>

using SDL2pp::Renderer;
using SDL2pp::Window;

Client::Client(EventHandler &&e_handler_rcv, ClientProtocol &&protocol_rcv)
    : e_handler(std::move(e_handler_rcv)), protocol(std::move(protocol_rcv)),
      receiver(responses, protocol), sender(protocol, commands) {}

void Client::run(int argc, char* argv[]) {

  receiver.start();
  sender.start();

  bool exited = main_menu(argc, argv);
  
  if(exited){
    stop();
    return;
  }

  ClientMessage message = {ActionType::CREATE_GAME, 5,      4, "pepe",
                           Direction::DEFAULT,      "holaa", CharacterAppearance{}};

  commands.push(message);

  bool was_intentional = false;
  Visual visual("Stardew Valley", SCREEN_WIDTH, SCREEN_HEIGHT);
  Loader loader(visual.get_renderer());

  // MapVisual map_visual;
  PlayerVisual p_visual;
  int width = SCREEN_WIDTH;
  int height = SCREEN_HEIGHT;
  Camera camera(width, height);

  int it = 1;
  Chronometer chrono;

  try {
    while (running) {
      // lógica de chronometer para calcular frames

      simulate_it(visual, camera, p_visual);

      it += chrono.sleep_and_calculate_it();
    }

  } catch (const std::exception &e) {
    if (was_intentional) {
      // se cierra ventana y sale en silencio
      return;
    }
    std::cerr << e.what() << "\n";
  }
}

void Client::simulate_it(Visual &visual, Camera &camera,
                         PlayerVisual &p_visual) {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (e_handler.is_disconnect(event)) {
      was_intentional = true;
      running = false;
    }

    ClientMessage msg = e_handler.get_next_event(event, visual, camera);

    if (msg.type != ActionType::NULL_ACTION) {
      commands.push(msg);
    }
  }

  // Hilo receiver se encarga de traer las respuestas del server.
  ServerMessage response;
  while (responses.try_pop(response)) {
    players = response.players;
  }

  visual.get_renderer().SetDrawColor(0, 0, 0, 255);

  visual.clean();

  for (const PlayerState &p : players) {
    p_visual.draw(visual.get_renderer(), p, camera);
    camera.update_pos(p.x, p.y, 32, 32);
  }

  visual.present();
}

void Client::stop() {
  responses.close();
  commands.close();

  receiver.stop();
  sender.stop();

  try {
    protocol.shutdown();

  } catch (const std::exception &) {
  }
}

bool Client::main_menu(int argc, char *argv[]) {
  QApplication app(argc, argv);

  MainWindow menu(responses, commands);
  menu.show();

  app.exec();

  return menu.has_exited();
}

Client::~Client() {
  stop();
  receiver.join();
  sender.join();
}

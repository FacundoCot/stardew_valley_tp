#include "client.h"
#include "../common_src/chronometer.h"
#include "../common_src/custom_error.h"
#include "../common_src/define.h"
#include "../common_src/messages.h"
#include "../common_src/queue.h"
#include "camera.h"
#include "eventhandler.h"
#include "loader.h"
#include "map_visual.h"
#include "player_visual.h"
#include "inventory.h"
#include "visual.h"
#include "clock.h"
#include "health_energy.h"

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>
#include <utility>

using SDL2pp::Renderer;
using SDL2pp::Window;

Client::Client(EventHandler &&e_handler_rcv, ClientProtocol &&protocol_rcv)
    : e_handler(std::move(e_handler_rcv)), protocol(std::move(protocol_rcv)),
      receiver(responses, protocol), sender(commands, protocol),
      camera(/*SCREEN_WIDTH, SCREEN_HEIGHT*/) {}

void Client::run() {
  receiver.start();
  sender.start();

  bool was_intentional = false;
  
  Visual visual("Stardew Valley", SCREEN_WIDTH, SCREEN_HEIGHT);
  Loader loader(visual.get_renderer());
  PlayerVisual p_visual(visual.get_renderer(), loader);
  Button btn("Terminar dia", visual, 200, 50);
  

  loader.load_everything();
  HealthEnergy h_and_e(loader);
  Clock clock(loader);
  Inventory inventory(loader);

  int it = 1;
  Chronometer chrono;
  
  try {
    while (running) {
      // lógica de chronometer para calcular frames

      simulate_it(clock, visual, btn, p_visual, inventory, loader, h_and_e, it);

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

void Client::simulate_it(Clock& clock, Visual& visual, Button& btn, 
  PlayerVisual& p_visual, Inventory& inventory, Loader& loader, HealthEnergy& h_and_e, int& it) {

  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (e_handler.is_disconnect(event)) {
      was_intentional = true;
      running = false;
    }

    ClientMessage msg = e_handler.get_next_event(event, visual, camera, current_state, btn);

    if (msg.type != ActionType::NULL_ACTION) {
        commands.push(msg);
    }
  }

  // Hilo receiver se encarga de traer las respuestas del server.
  ServerMessage response;
  while (responses.try_pop(response)) {

      if (response.type == MessageType::GAME_STATE) {
          update(response, clock, inventory, h_and_e);
      }
  }

  render(clock, visual, btn, p_visual, inventory, loader, h_and_e, it);
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

void Client::render(Clock& clock, Visual& visual, Button& btn, 
  PlayerVisual& p_visual, Inventory& inventory, Loader& loader, HealthEnergy& h_and_e, int& it) {

  visual.get_renderer().SetDrawColor(0, 0, 0, 255);
  visual.clean();

  for (const PlayerState &p : players) {
    p_visual.draw(p, camera, it);
  }

  p_visual.draw(self, camera, it);

  clock.draw(visual);
  inventory.draw_inventory(visual, loader);
  h_and_e.draw(visual);

  if (current_state == ClientState::DAY_ENDED) {
    visual.draw_black_overlay();
    btn.draw(visual);
  }
  else if (current_state == ClientState::WAITING_OTHER_PLAYERS) {
    visual.draw_black_overlay();
    visual.draw_text_centered("Esparando a los demas jugadores...", r, g, b);
  }

  visual.present();
}

void Client::update(ServerMessage& response, Clock& clock, Inventory& inventory, HealthEnergy& h_and_e) {
  if (!response.players.empty()) {
    players = response.players;
  }
  self = response.self_player;
  camera.update_pos(response.self_player, 32, 32);

  clock.update_from_server(response.time);
  inventory.update_from_server(response);
  h_and_e.update(response);

  if (current_state == ClientState::PLAYING) {
    if(clock.is_finished(response.time)) {
      current_state = ClientState::DAY_ENDED;
    }
  }
  else if (current_state == ClientState::WAITING_OTHER_PLAYERS){
      if(!clock.is_finished(response.time)) {
        current_state = ClientState::PLAYING;
      }
  }
}


Client::~Client() {
  stop();
  receiver.join();
  sender.join();
}

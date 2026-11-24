#include "client.h"
#include "../common_src/chronometer.h"
#include "../common_src/custom_error.h"
#include "../common_src/define.h"
#include "../common_src/messages.h"
#include "../common_src/queue.h"
#include "../common_src/world_serialization.h"
#include "camera.h"
#include "eventhandler.h"
#include "loader.h"
#include "map_visual.h"
#include "player_visual.h"
#include "inventory.h"
#include "visual.h"
#include "clock.h"
#include "health_energy.h"
#include "appearance_translator.h"
#include "map_constants.h"


#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>
#include <utility>

using SDL2pp::Renderer;
using SDL2pp::Window;

Client::Client(Queue<ServerMessage>& responses,ClientProtocol &protocol_rcv)
    : responses(responses), protocol(protocol_rcv),
      sender(commands, protocol), camera(), visual("Stardew Valley", SCREEN_WIDTH, SCREEN_HEIGHT),
      loader(visual.get_renderer()), p_visual(visual.get_renderer(), loader), h_and_e(loader), clock(loader),
      inventory(loader), menu_game(visual, loader) ,e_handler(menu_game)  {}

void Client::run() {
  sender.start();

  bool world_loaded = false;

  SerializedWorld world;
  ServerMessage first_msg;

  while(!world_loaded) {
    first_msg = responses.pop();
    if (first_msg.type == MessageType::WORLD_INFO) {
      world = decode_world(first_msg.world_info);
      world_loaded = true;
    } 
    
  }

  if (!world_loaded) {
    throw std::runtime_error("Error at running Client: cannot load world!");
  }

  camera.set_dimensions(visual.get_renderer().GetOutputWidth(), visual.get_renderer().GetOutputHeight());

  for (const auto& zone : world.zones) {
    if(zone.id.kind == ZoneId::Kind::FARM) {
      int width = zone.width*TILE_SIZE;
      int height = zone.height*TILE_SIZE;
      camera.set_dimensions_map(width, height );
    }
  }

  MapVisual map_visual(visual, loader, std::move(world));
  SDL2pp::Texture& btn_confirm_day_t = loader.get_texture("btn_confirm_day");
  int btn_w = static_cast<int>(btn_confirm_day_t.GetWidth()*0.1f);
  int btn_h = static_cast<int>(btn_confirm_day_t.GetHeight()*0.1f);
  int btn_x =( visual.get_window().GetWidth() - btn_w) / 2 ;
  int btn_y = (visual.get_window().GetHeight() - btn_h) / 2;
  Button btn_confirm_day(btn_confirm_day_t, btn_x, btn_y, btn_w, btn_h);

  int it = 1;
  Chronometer chrono;
  
  try {
    while (running) {
      // lógica de chronometer para calcular frames

      simulate_it(btn_confirm_day, it, map_visual);

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

void Client::simulate_it(Button& btn, int& it, MapVisual& m_visual) {

  SDL_Event event;
  while (SDL_PollEvent(&event)) {

    ClientMessage msg = e_handler.get_next_event(event, visual, camera, current_state, btn);

    if (e_handler.is_disconnect(event) || msg.type == ActionType::DISCONNECT) {
      was_intentional = true;
      running = false;
    }

    if (msg.type != ActionType::NULL_ACTION) {
        commands.push(msg);
    }
  }

  // Hilo receiver se encarga de traer las respuestas del server.
  ServerMessage response;
  while (responses.try_pop(response)) {

      if (response.type == MessageType::GAME_STATE) {
          update(response);
      }
  }

  render(btn, it, m_visual);
}

void Client::stop() {
  commands.close();
  sender.stop();
}

void Client::render(Button& btn, int& it, MapVisual& m_visual) {

  visual.get_renderer().SetDrawColor(0, 0, 0, 255);
  visual.clean();

  m_visual.draw_map(zone_player, camera);

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
    visual.draw_text_centered("Esperando a los demas jugadores...", r, g, b);
  }
  
  menu_game.display_menu();
  visual.present();
}

void Client::update(ServerMessage& response) {

  players = response.players;
  
  self = response.self_player;
  camera.update_pos(self, BODY_WIDTH*2, BODY_HEIGHT*2);
  zone_player = response.zone;

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
  sender.join();
}

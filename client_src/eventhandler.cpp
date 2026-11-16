#include "eventhandler.h"
#include "../common_src/messages.h"
#include "camera.h"
#include "visual.h"
#include "button.h"

#include <SDL2/SDL.h>
#include <stdexcept>

ClientMessage EventHandler::get_event_to_protocol(const SDL_Event &e,
                                                  ClientMessage &msg) {
  if (is_select_slot(e)) {return select_slot(e, msg);}

  switch (e.key.keysym.sym) {
  case (SDLK_w):
    msg.type = ActionType::MOVE;
    msg.direction = Direction::UP;
    return msg;

  case (SDLK_s):
    msg.type = ActionType::MOVE;
    msg.direction = Direction::DOWN;
    return msg;

  case (SDLK_a):
    msg.type = ActionType::MOVE;
    msg.direction = Direction::LEFT;
    return msg;

  case (SDLK_d):
    msg.type = ActionType::MOVE;
    msg.direction = Direction::RIGHT;
    return msg;

  default:
    msg.type = ActionType::NULL_ACTION;
    return msg;
  }
}

ClientMessage EventHandler::get_next_event(const SDL_Event &e, Visual &visual,
                                           Camera &camera, ClientState& state, Button& btn) {
  ClientMessage msg;
  if (e.type == SDL_QUIT) {
    msg.type = ActionType::DISCONNECT;
    return msg;
  }
  switch (state) {
    case (ClientState::DAY_ENDED):
      return get_mouse_event(e, btn, state);

    case (ClientState::WAITING_OTHER_PLAYERS):
      msg.type = ActionType::NULL_ACTION;
      return msg;

    case (ClientState::PLAYING):
      if(e.type == SDL_KEYDOWN) {

        change_window_dimension(e, visual, camera);

        return get_event_to_protocol(e, msg);
      }
      msg.type = ActionType::NULL_ACTION;
      return msg;

    default:
      return {};

  }
 
}

bool EventHandler::is_disconnect(const SDL_Event &e) {
  return e.type == SDL_QUIT;
}

void EventHandler::change_window_dimension(const SDL_Event& e, Visual& visual, Camera& camera) {  
  if (e.key.keysym.sym == SDLK_F11) {
    SDL2pp::Window &w = visual.get_window();
    bool fulscreen = visual.get_is_fulscreen();

    if (e.key.repeat == 0) {
      if (fulscreen) {
        w.SetFullscreen(0);
        w.SetSize(1280, 720);
        w.SetPosition(SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
        camera.set_dimensions(1280, 720);
      } else {
        w.SetFullscreen(SDL_WINDOW_FULLSCREEN_DESKTOP);
        camera.set_dimensions(w.GetWidth(), w.GetHeight());
      }
      visual.set_is_fulscreen();
    }
  } 
}

bool EventHandler::is_select_slot(const SDL_Event& e) {
    return e.key.keysym.sym >= (SDLK_0) &&  e.key.keysym.sym <= (SDLK_8);
}

ClientMessage EventHandler::select_slot(const SDL_Event& e, ClientMessage& msg) {
  msg.type = ActionType::SELECT_SLOT;
  msg.slot_index = e.key.keysym.sym - SDLK_0;
  return msg;
}

ClientMessage EventHandler::get_mouse_event(const SDL_Event &e, Button& btn, ClientState& state) {
  ClientMessage msg;
  if (e.type == SDL_MOUSEBUTTONDOWN) {
    int mouse_x = static_cast<int>(e.button.x);
    int mouse_y = static_cast<int>(e.button.y);

    if(btn.is_clicked(mouse_x, mouse_y)) {
      msg.type = ActionType::CONFIRM_NEXT_DAY;
      state = ClientState::WAITING_OTHER_PLAYERS;
      return msg;
    }
  }
  msg.type = ActionType::NULL_ACTION;
  return msg;
}
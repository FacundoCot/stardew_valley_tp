#include "eventhandler.h"
#include "../common_src/messages.h"

#include <SDL2/SDL.h>
#include <stdexcept>

ClientMessage EventHandler::get_event_to_protocol(const SDL_Event &e,
                                                  ClientMessage &msg) {
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
    throw std::runtime_error("There is no event possible!\n");
  }
}

ClientMessage EventHandler::get_next_event(const SDL_Event &e) {
  ClientMessage msg;
  if (e.type == SDL_QUIT) {
    msg.type = ActionType::DISCONNECT;
    return msg;

  } else if (e.key.keysym.sym == SDLK_ESCAPE) {
    // si se aprieta el boton de salir se cierra
  }

  return get_event_to_protocol(e, msg);
}

bool EventHandler::is_disconnect(const SDL_Event &e) {
  return e.type == SDL_QUIT;
}

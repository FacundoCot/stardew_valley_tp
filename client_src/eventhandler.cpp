#include "eventhandler.h"
#include "../common_src/messages.h"
#include "camera.h"
#include "visual.h"

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
    msg.type = ActionType::NULL_ACTION;
    return msg;
  }
}

ClientMessage EventHandler::get_next_event(const SDL_Event &e, Visual &visual,
                                           Camera &camera) {
  ClientMessage msg;
  if (e.type == SDL_QUIT) {
    msg.type = ActionType::DISCONNECT;
    return msg;
  }
  if (e.type == SDL_KEYDOWN) {
    if (e.key.keysym.sym == SDLK_F11) {
      msg.type = ActionType::NULL_ACTION;
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
      return msg;
    }
  }

  return get_event_to_protocol(e, msg);
}

bool EventHandler::is_disconnect(const SDL_Event &e) {
  return e.type == SDL_QUIT;
}

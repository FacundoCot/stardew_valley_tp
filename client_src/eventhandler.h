#ifndef EVENTHANDLER_H
#define EVENTHANDLER_H

#include <SDL2/SDL.h>

#include "../common_src/messages.h"
#include "camera.h"
#include "visual.h"
#include "button.h"

class EventHandler {
private:
  ClientMessage get_event_to_protocol(const SDL_Event &e, ClientMessage &msg);
  ClientMessage select_slot(const SDL_Event& e, ClientMessage& msg);
  bool is_select_slot(const SDL_Event &e);

public:
  ClientMessage get_next_event(const SDL_Event &e, Visual &visual,
                               Camera &camera, ClientState& state, Button& btn);
  bool is_disconnect(const SDL_Event &e);
  void change_window_dimension(const SDL_Event &e, Visual& visual, Camera& camera);
  ClientMessage get_mouse_event(const SDL_Event &e, Button& btn, ClientState &state);
};

#endif

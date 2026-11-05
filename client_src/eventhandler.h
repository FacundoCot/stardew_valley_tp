#ifndef EVENTHANDLER_H
#define EVENTHANDLER_H

#include <SDL2/SDL.h>

#include "../common_src/messages.h"
#include "camera.h"
#include "visual.h"

class EventHandler {
private:
  ClientMessage get_event_to_protocol(const SDL_Event &e, ClientMessage &msg);

public:
  ClientMessage get_next_event(const SDL_Event &e, Visual &visual,
                               Camera &camera);
  bool is_disconnect(const SDL_Event &e);
};

#endif

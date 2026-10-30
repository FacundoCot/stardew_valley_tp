#ifndef CHRONOMETER_H
#define CHRONOMETER_H

#include <SDL2/SDL.h>

class Chronometer {
private:
public:
  uint32_t get_time_difference();
  void sleep();
};

#endif

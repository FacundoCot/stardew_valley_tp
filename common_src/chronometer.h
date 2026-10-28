#ifndef CHRONOMETER_H
#define CHRONOMETER_H

#include <SDL2/SDL.h>
#include <chrono>

class Chronometer {
private:
  std::chrono::_V2::steady_clock::time_point t1;

  int calculate_if_is_behind(double &rest);

public:
  Chronometer();

  int sleep_and_calculate_it();
  void sleep(double &rest);
};

#endif

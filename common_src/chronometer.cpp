#include <SDL2/SDL.h>

#include "chronometer.h"
#include <chrono>
#include <unistd.h>

#include "define.h"

Chronometer::Chronometer() : t1(std::chrono::steady_clock::now()) {}

void Chronometer::sleep(double &rest) { SDL_Delay(rest); }

int Chronometer::calculate_if_is_behind(double &rest) {
  double behind = -rest;
  rest = RATE - (static_cast<uint32_t>(behind) % static_cast<uint32_t>(RATE));
  double lost = behind + rest;
  std::chrono::duration<double, std::milli> ms_perdidos(lost);
  t1 += std::chrono::duration_cast<std::chrono::steady_clock::duration>(
      ms_perdidos);
  int it = static_cast<int>(lost / RATE);
  return it;
}

int Chronometer::sleep_and_calculate_it() {
  auto t2 = std::chrono::steady_clock::now();
  const std::chrono::duration<double, std::milli> diff = t2 - t1;
  // count obtiene el float del objeto duration.
  double rest = RATE - diff.count();
  int it = 0;
  if (rest < 0) {
    it = calculate_if_is_behind(rest);
  }
  sleep(rest);
  std::chrono::duration<double, std::milli> ms_rate(RATE);
  t1 +=
      std::chrono::duration_cast<std::chrono::steady_clock::duration>(ms_rate);
  it++;
  return it;
}

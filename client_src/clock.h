#ifndef CLOCK_H
#define CLOCK_H

#include "loader.h"
#include "visual.h"
#include <SDL2pp/Texture.hh>
#include "../common_src/messages.h"


class Clock {
private:
SDL2pp::Texture& clock_texture;
float current_angle;

int x_clock = 0;
int y_clock = 0;
int w_clock = 72;
int h_clock = 59;

int x_arrow = 72;
int y_arrow = 0;
int w_arrow = 7;
int h_arrow = 19;

int x_day_rectangle = 27;
int y_day_rectangle = 4;
int w_day_rectangle = 40;
int h_day_rectangle = 11;


uint16_t day;
uint8_t hour;
uint8_t minutes;

const uint8_t scale = 2;

public:
explicit Clock(Loader& loader);

void draw(Visual& visual);
void update_from_server(TimeState &time);
bool is_finished(TimeState &time);

};


#endif
#ifndef HEALTH_ENERGY_H
#define HEALTH_ENERGY_H

#include "../common_src/messages.h"
#include "visual.h"
#include "loader.h"

class HealthEnergy {
private:
SDL2pp::Texture& texture;

int health;
int energy;

int max_health;
int max_energy;

const uint16_t padding = 10;
const uint16_t margin = 20;
const uint16_t scale = 7;

int src_x = 16;
int src_y = 102;

int width = 16;
int heigth = 3;

public:

HealthEnergy(Loader& loader);

void update(ServerMessage& msg);
void draw(Visual& visual); 

};



#endif


#ifndef DIMENSIONS_H
#define DIMENSIONS_H   
#include "visual.h"

struct DimensionsStruct {
    int x;
    int y;
    int w;
    int h;
};

class Dimensions {
private:
const uint8_t scale = 2;

const int w_clock = 72*scale;
const int h_clock = 59*scale;
const int margin = 20;

const uint8_t scale_stick = 7;

const int w_stick = 16;
const int h_stick = 3;
const int padding = 10;

public:

DimensionsStruct get_clock_dimensions(Visual& visual);
DimensionsStruct get_health_stick(Visual& visual);
};




#endif
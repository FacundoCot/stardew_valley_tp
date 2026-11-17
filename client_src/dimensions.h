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

public:

DimensionsStruct get_clock_dimensions(Visual& visual);

};




#endif
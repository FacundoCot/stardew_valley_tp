#include "dimensions.h"


DimensionsStruct Dimensions::get_clock_dimensions(Visual& visual) {
    DimensionsStruct clock;
    clock.x = visual.get_window().GetWidth() - margin;
    clock.y = margin + h_clock;
    clock.w = w_clock;
    clock.h = h_clock;
    
    return clock;
}
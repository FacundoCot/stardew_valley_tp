#include "dimensions.h"


DimensionsStruct Dimensions::get_clock_dimensions(Visual& visual) {
    DimensionsStruct clock;
    clock.x = visual.get_window().GetWidth() - margin;
    clock.y = margin + h_clock;
    clock.w = w_clock;
    clock.h = h_clock;
    
    return clock;
}

DimensionsStruct Dimensions::get_health_stick(Visual& visual) {
    DimensionsStruct energy_stick;
    DimensionsStruct clock = get_clock_dimensions(visual);
    energy_stick.y = clock.y + padding*2 + h_stick*scale_stick*2;
    energy_stick.x = 0;
    energy_stick.w = 0;
    energy_stick.h = 0;

    return energy_stick;
}
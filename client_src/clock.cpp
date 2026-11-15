#include "clock.h"
#include "loader.h"
#include "visual.h"

#include <SDL2pp/Window.hh>

Clock::Clock(Loader& loader) :  clock_texture(loader.get_texture("clock")),current_angle(-135.0), day(1), hour(), minutes() {}

void Clock::draw(Visual& visual) {
    int margin = 20;

    int dest_w = w_clock * scale;
    int dest_h = h_clock * scale;

    int dest_x = visual.get_window().GetWidth() - dest_w - margin;
    int dest_y = margin;

    visual.draw(clock_texture, x_clock, y_clock, h_clock, w_clock, dest_x, dest_y, dest_w, dest_h);

    int screen_x_arr = dest_x + (22*scale);
    int screen_y_arr = dest_y + (20*scale);


    int w_arrow_final = w_arrow * scale;
    int h_arrow_final = h_arrow * scale;

    int half_arrow = w_arrow_final / 2;

    int pos_arrow_x = screen_x_arr - half_arrow;
    int pos_arrow_y = screen_y_arr - h_arrow_final;

    visual.draw_rotate(clock_texture, x_arrow, y_arrow, h_arrow, w_arrow, 
                        pos_arrow_x, pos_arrow_y, w_arrow_final, h_arrow_final, 
                        current_angle, half_arrow, h_arrow_final);


    std::string day_info = "Dia " + std::to_string(this->day);
    std::string min_str = (this->minutes < 10 ? "0" : "") + std::to_string(this->minutes);
    std::string hour_str = std::to_string(this->hour) + ":" + min_str;
    
    int x_day = 17 + dest_x + (x_day_rectangle*scale);
    int y_day = dest_y + (y_day_rectangle*scale);

    int x_hour = 20 + dest_x + (27*scale);
    int y_hour = dest_y + (27*scale);
    
    uint8_t r = 70;
    uint8_t g = 40;
    uint8_t b = 20;

    visual.draw_text(day_info, x_day, y_day, r, g, b);
    visual.draw_text(hour_str, x_hour, y_hour, r, g, b);

}

void Clock::update_from_server(TimeState &time) {

    this->day = time.day;

    uint8_t normalized_hour = time.hour;
    if (normalized_hour < 6) {
        normalized_hour += 24;
    }

    float total_min = 60.0f * 20.0f;
    float elapsed_min = ((normalized_hour - 6) * 60) + time.minute;

    float total_advanced = elapsed_min/total_min;

    if (total_advanced > 1.0f ) total_advanced = 1.0f;
    if (total_advanced < 0.0f) total_advanced = 0.0f;

    //el reloj es una semicircunferencia en, la flecha va desde los
    // -135 a -45 grados, como total_advanced va de 0 a 1 entonces
    // cuando sea 1 y sea lo mas tarde posible, entonces esa cuenta da 90
    // y cuando es 0 da -90.
    float initial_angle = -135.0;
    float total = 90.0;
    this->current_angle = (total_advanced*total) + initial_angle;
    this->hour = time.hour;
    this->minutes = time.minute;

}

bool Clock::is_finished(TimeState& time) {
    if (time.finished) {
        this->hour = 6;
        this->minutes = 0;
        return true;
    }
    return false;
}
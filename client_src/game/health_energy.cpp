#include "health_energy.h"
#include "loader.h"
#include "visual.h"
#include "dimensions.h"
#include <SDL2pp/Renderer.hh>


HealthEnergy::HealthEnergy(Loader& loader) : texture(loader.get_texture("Inventory")) {}


void HealthEnergy::draw(Visual& visual) {
    Dimensions dim;
    DimensionsStruct end_clock = dim.get_clock_dimensions(visual);

    int dest_w = width*scale;
    int dest_h = heigth*scale;

    int dest_x_e = end_clock.x - dest_w;
    int dest_y_e = end_clock.y + padding;

    int dest_x_h = dest_x_e;
    int dest_y_h = dest_y_e + dest_h + padding;

    visual.draw(texture, src_x, src_y, heigth, width, dest_x_h, dest_y_h, dest_w, dest_h);
    visual.draw(texture, src_x, src_y, heigth, width, dest_x_e, dest_y_e, dest_w, dest_h);


    SDL2pp::Renderer& renderer = visual.get_renderer();

    float total_h = static_cast<float>(health/max_health);
    float total_e = static_cast<float>(energy / max_energy);
    
    int border = 1 * scale;
    int w = dest_w - (2*border);
    int h = dest_h - (2*border);

    int fill_w_h = static_cast<int>(w*total_h);
    int fill_w_e = static_cast<int>(w*total_e);

    if (fill_w_h > 0) {
        renderer.SetDrawColor(220, 50, 50, 255);
        renderer.FillRect(SDL2pp::Rect(dest_x_h + border, dest_y_h + border, fill_w_h, h));
    }

    if (fill_w_e > 0) {
        renderer.SetDrawColor(80, 200, 60, 255);
        renderer.FillRect(SDL2pp::Rect(dest_x_e + border, dest_y_e + border, fill_w_e, h));
    } 
}


void HealthEnergy::update(ServerMessage& msg) {
    this->max_energy = msg.self_player.max_energy;
    this->max_health = msg.self_player.max_health;
    
    this->energy = msg.self_player.energy;
    this->health = msg.self_player.health;
}
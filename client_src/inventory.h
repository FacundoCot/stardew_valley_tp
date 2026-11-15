#ifndef INVENTORY_H
#define INVENTORY_H

#include "SDL2pp/Texture.hh"
#include "../common_src/messages.h"
#include "loader.h"
#include "visual.h"
#include <vector>

struct InventorySlot {

    uint8_t id;
    bool is_selected;

    bool is_empty = true;
    ItemType type;
    Quality quality;
    uint8_t quantity;
};

class Inventory {
private:
SDL2pp::Texture &texture_inventory;
std::vector<InventorySlot> slots;

uint32_t coins;

int x_inv_slot = 0;
int y_inv_slot = 64;
int w_inv_slot = 15;
int h_inv_slot = 15;

int scale = 3;
int margin = 20;

SDL2pp::Texture &get_texture(Loader& loader, ItemType& type);

public:

Inventory(Loader& loader);

void update_position(int& screen_w, int& screen_h);
void draw_inventory(Visual& visual, Loader& loader);
void update_from_server(const ServerMessage &msg);
};



#endif
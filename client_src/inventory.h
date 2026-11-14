/*#ifndef INVENTORY_H
#define INVENTORY_H

#include "SDL2pp/Texture.hh"
#include "../common_src/messages.h"
#include "loader.h"
#include <vector>

struct InventorySlot {

    uint8_t id;
    SDL2pp::Rect box;
    bool is_selected;

    bool is_empty = true;
    ItemType type;
    Quality quality;
    uint8_t quantity;
};


class Inventory {
private:
std::vector<InventorySlot> slots;
SDL2pp::Texture &texture_inventory;

public:

Inventory(Loader &loader);

void update_position(int& screen_w, int& screen_h);
void update_from_server(const ServerMessage &msg);
};



#endif*/
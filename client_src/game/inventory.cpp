#include "inventory.h"
#include "loader.h"
#include "../../common_src/messages.h"
#include "../../common_src/custom_error.h"
#include "visual.h"
#include <format>

Inventory::Inventory(Loader& loader) :  texture_inventory(loader.get_texture("Inventory")), slots(9) {}


void Inventory::draw_inventory(Visual& visual, Loader& loader) {
    int total_width = w_inv_slot*scale*slots.size();
    int dest_w = w_inv_slot*scale;
    int dest_h = h_inv_slot*scale;

    for (uint8_t i=0; i < slots.size(); i++) {

        int dest_x = visual.get_window().GetWidth() / 2 - (total_width / 2) + i*dest_w;
        int dest_y = visual.get_window().GetHeight() - dest_h - margin;

        visual.draw(texture_inventory, x_inv_slot, y_inv_slot, h_inv_slot, w_inv_slot, dest_x, dest_y, dest_h, dest_w);
        if (!slots[i].is_empty) {
            SDL2pp::Texture &texture = get_texture(loader, slots[i].type);
            int padding = 10;

            int dest_item_x = dest_x + padding;
            int dest_item_y = dest_y + padding;
            int dest_item_h = dest_h - (padding*2);
            int dest_item_w = dest_w - (padding*2);

            int src_item_x = 0;
            int src_item_y = 0;
            int src_item_w = texture.GetWidth();
            int src_item_h = texture.GetHeight();

            visual.draw(texture, src_item_x, src_item_y, src_item_h, src_item_w, dest_item_x, 
                    dest_item_y, dest_item_h, dest_item_w);

            if (slots[i].is_selected) {
                draw_selected(visual, dest_item_x, dest_item_y, dest_item_w, dest_item_h);
            }

        }
    }
    draw_coins(visual);
}

void Inventory::update_from_server(const ServerMessage &response) {
    for (auto &slot : slots) {
        slot.is_empty = true;
    }

    this->coins = response.inventory.coins;
    slots[response.inventory.selected_slot].is_selected = true;

    for (const auto& server_slot : response.inventory.slot_vector) {
        uint8_t i = server_slot.index;
        if (i < slots.size()) {
            slots[i].type = server_slot.type;
            slots[i].quality = server_slot.quality;
            slots[i].quantity = server_slot.quantity;
            slots[i].is_empty = false;
            if (slots[i].is_selected && i != response.inventory.selected_slot) {
                slots[i].is_selected = false;
            }
        }
    }
}


void Inventory::draw_coins(Visual& visual) {
    int coins_x = visual.get_window().GetWidth() - 2*72 + 50 ;
    int coins_y = 20 + 47*2;

    uint8_t r = 70;
    uint8_t g = 40;
    uint8_t b = 20;

    std::string coins_text = std::to_string(this->coins);
    visual.draw_text_cached("inventory_coins",coins_text, coins_x, coins_y, r, g, b);
}


void Inventory::draw_selected(Visual& visual, int& x, int& y, int& w, int& h) {
    SDL2pp::Renderer& renderer = visual.get_renderer();

    renderer.SetDrawBlendMode(SDL_BLENDMODE_BLEND);

    renderer.SetDrawColor(255,255,255, 150);

    renderer.FillRect(SDL2pp::Rect(x, y, w, h));

    renderer.SetDrawBlendMode(SDL_BLENDMODE_NONE);
}

SDL2pp::Texture& Inventory::get_texture(Loader& loader, ItemType& type) {
    switch(type) 
    {
    case ItemType::PICKAXE:
        return loader.get_texture("pickaxe");
    
    case ItemType::WATERING_CAN:
        return loader.get_texture("watering_can");
          
    case ItemType::SHOVEL:
        return loader.get_texture("shovel");
        
    case ItemType::SWORD:
        return loader.get_texture("sword");
        
    case ItemType::STRAWBERRY:
        return loader.get_texture("strawberry");
        
    case ItemType::ONION:
        return loader.get_texture("onion");
        
    case ItemType::POTATO:
        return loader.get_texture("potato");
        
    case ItemType::PUMPKIN:
        return loader.get_texture("pumpkin");
        
    case ItemType::STRAWBERRY_SEED:
        return loader.get_texture("strawberry_seed");
        
    case ItemType::ONION_SEED:
        return loader.get_texture("onion_seed");
        
    case ItemType::POTATO_SEED:
        return loader.get_texture("potato_seed");
        
    case ItemType::PUMPKIN_SEED:
        return loader.get_texture("pumpkin_seed");
        
    case ItemType::COAL:
        return loader.get_texture("coal");
        
    case ItemType::LAPIS_LAZULI:
        return loader.get_texture("lapislazuli");
        
    case ItemType::SILVER:
        return loader.get_texture("silver");
        
    case ItemType::GOLD:
        return loader.get_texture("gold");

    case ItemType::MILK:
        return loader.get_texture("milk");
        
    case ItemType::EGG:
        return loader.get_texture("egg");
        
    case ItemType::BED:
        return loader.get_texture("bed");
        
    case ItemType::CHEST:
        return loader.get_texture("chest");

    default:
        throw ItemNotFound("Error at inventory. There is no item with that code!");
    }
}

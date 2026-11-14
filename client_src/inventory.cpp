/*#include "inventory.h"
#include "loader.h"
#include "../common_src/messages.h"

Inventory::Inventory(Loader &loader_rcv) : texture_inventory(loader_rcv.get_texture("Inventory")) {}

void Inventory::update_from_server(const ServerMessage &response) {
    for (auto &slot : slots) {
        slot.is_empty = true;
    }

    for (const auto& server_slot : response.inventory.slot_vector) {
        int i = server_slot.index;
        if (i >= 0 && i < slots.size()) {
            slots[i]
        }
    }
}*/
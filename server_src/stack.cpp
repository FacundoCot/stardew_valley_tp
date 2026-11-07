#include "stack.h"
#include <stdexcept>

Stack::Stack(Item item, int quantity)
    : item(item), quantity(quantity) {
        int max = item.is_stackable() ? 50 : 1;
        if (quantity <= 0 || quantity > max) {
            throw std::invalid_argument("Cantidad invalida");
        }
}

Item Stack::get_item() const {
    return item;
}

int Stack::get_quantity() const {
    return quantity;
}

bool Stack::set_quantity(int quantity) {
    int max = item.is_stackable() ? 50 : 1;
    if (quantity <= max && quantity >= 1) {
        this->quantity = quantity;
        return true;
    }
    return false;
}


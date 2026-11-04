#include "stack.h"
#include <stdexcept>

Stack::Stack(Item item, unsigned int quantity)
    : item(item), quantity(quantity) {
    if (quantity == 0 || quantity > 50) {
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
    if (quantity <= 50 && quantity >= 1) {
        this->quantity = quantity;
        return true;
    }
    return false;
}


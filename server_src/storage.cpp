#include "storage.h"

Storage::Storage(int capacity) : capacity(capacity) {}

int Storage::add_item(const Item& item, int quantity) {
    int max_quantity = item.is_stackable() ? 50 : 1;
    
    for (Stack& stack : stacks) {
        if (max_quantity == 1 || quantity == 0) {
            break;
        }
        if (stack.get_item() == item) {
            int current_quantity = stack.get_quantity();
            if (current_quantity < max_quantity) {
                int space_left = max_quantity - current_quantity;
                int to_add = std::min(space_left, quantity);
                
                if (stack.set_quantity(current_quantity + to_add)) {
                    quantity -= to_add;

                }
            }
        }
    }

    while (quantity > 0 && stacks.size() < capacity) {
        int to_add = std::min(max_quantity, quantity);
        stacks.emplace_back(item, to_add);
        quantity -= to_add;
    }
    return quantity;
}

bool Storage::remove_item(const Item& item, int quantity) {
    // Si no hay suficientes objetos, no quitamos nada.
    if (count_item(item) < quantity) {
        return false;
    }

    auto it = stacks.begin();

    while (it != stacks.end() && quantity > 0) {
        if (it->get_item() == item) {
            int current = it->get_quantity();

            if (current <= quantity) {
                // Se vacia la pila y se elimina.
                quantity -= current;
                it = stacks.erase(it);
            } else {
                // Solo se quita una parte de la pila.
                it->set_quantity(current - quantity);
                quantity = 0;
            }
        } else {
            ++it;
        }
    }

    return true;
}

int Storage::count_item(const Item& item) const {
    int total = 0;

    for (const Stack& stack : stacks) {
        if (stack.get_item() == item) {
            total += stack.get_quantity();
        }
    }

    return total;
}

const std::vector<Stack>& Storage::get_stacks() const {
    return stacks;
}
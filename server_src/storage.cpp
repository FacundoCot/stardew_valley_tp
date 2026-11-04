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

    while (quantity > 0 && stacks.size() < static_cast<size_t>(capacity)) {
        int to_add = std::min(max_quantity, quantity);
        stacks.emplace_back(item, to_add);
        quantity -= to_add;
    }
    return quantity;
}

std::optional<Stack> Storage::remove_item(int index, RemoveMode mode) {
    // Comprobar que exista la pila seleccionada.
    if (index < 0 || static_cast<size_t>(index) >= stacks.size()) {
        return std::nullopt;
    }

    Stack& stack = stacks[index];

    int current = stack.get_quantity();
    int to_remove = 0;

    switch (mode) {
        case RemoveMode::ONE:
            to_remove = 1;
            break;

        case RemoveMode::HALF:
            to_remove = (current + 1) / 2;
            break;

        case RemoveMode::ALL:
            to_remove = current;
            break;

        default:
            return std::nullopt;
            break;
    }

    // Guardar lo que vamos a retirar
    Stack removed(stack.get_item(), to_remove);

    if (to_remove == current) {
        // Se elimina la pila completa
        stacks.erase(stacks.begin() + index);
    } else {
        // Se descuenta solamente lo retirado
        stack.set_quantity(current - to_remove);
    }

    return removed;
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
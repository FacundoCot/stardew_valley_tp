
#include "storage.h"

#include <algorithm>
#include <stdexcept>

Storage::Storage(int capacity) {
    // se hace asi para chequear que no se pase una capacidad
    // negativa
    if (capacity <= 0) {
        throw std::invalid_argument("Capacidad invalida");
    }

    stacks.resize(static_cast<size_t>(capacity));
}

int Storage::quantity_to_remove(int current, RemoveMode mode) {
    switch (mode) {
        case RemoveMode::ONE:
            return 1;

        case RemoveMode::HALF:
            return (current + 1) / 2;

        case RemoveMode::ALL:
            return current;

        default:
            return 0;
    }
}

// agregar objetos sin especificar casilleros
int Storage::add_item(const Item& item, int quantity) {
    if (quantity <= 0) {
        return quantity;
    }

    const int max_quantity = item.is_stackable() ? 50 : 1;

    // Primero completamos las pilas existentes
    if (item.is_stackable()) {
        for (auto& slot : stacks) {
            if (quantity == 0) {
                break;
            }

            // me fijo si el slot esta desocupado o tiene otro
            // item diferente al que busco
            if (!slot || !(slot->get_item() == item)) {
                continue;
            }

            const int current = slot->get_quantity();
            const int to_add = std::min(max_quantity - current, quantity);

            if (to_add > 0) {
                slot->set_quantity(current + to_add);
                quantity -= to_add;
            }
        }
    }

    // despues ocupamos los casilleros vacios
    for (auto& slot : stacks) {
        if (quantity == 0) {
            break;
        }
        if (!slot) {
            const int to_add = std::min(max_quantity, quantity);
            // crea directamente un stack dentro del vector
            slot.emplace(item, to_add);
            quantity -= to_add;
        }
    }

    return quantity;
}

// verifica si podemos agregar todos los objetos, no modifica el inventario
// util para saber si entra la compra por ejemplo 
bool Storage::can_add_item(const Item& item, int quantity) const {
    if (quantity <= 0) {
        return false;
    }

    const int max_quantity = item.is_stackable() ? 50 : 1;
    int available = 0;

    for (const auto& slot : stacks) {
        if (!slot) {
            available += max_quantity;
        } else if (item.is_stackable() && slot->get_item() == item) {
            available += max_quantity - slot->get_quantity();
        }

        if (available >= quantity) {
            return true;
        }
    }

    return false;
}

//coloca una pila en un casillero especifico, lo usamos para reconstruir inventarrio
bool Storage::place_item(int index, const Item& item, int quantity) {
    if (index < 0 || static_cast<size_t>(index) >= stacks.size()) {
        return false;
    }

    const int max_quantity = item.is_stackable() ? 50 : 1;

    // si esta ocupado, o si la cantidad es negativa, o si la cantidad es mayor
    // a la capacidad maxima, devolvemos false
    if (stacks[index].has_value() || quantity <= 0 || quantity > max_quantity) {
        return false;
    }

    //creo un stack nuevo con mi item a poner
    stacks[index].emplace(item, quantity);
    return true;
}


// retira items de un casillero en especifico, se le especifica si ONE, HALF, ALL
// nos sirve para transferir obejtos, guardarlos en cofres y eso
std::optional<Stack> Storage::remove_item(int index, RemoveMode mode) {
    if (index < 0 || static_cast<size_t>(index) >= stacks.size()  || !stacks[index].has_value()) {
        return std::nullopt;
    }

    Stack& stack = *stacks[index];

    const int current = stack.get_quantity();
    const int to_remove = quantity_to_remove(current, mode);

    // creo el stack a devolver con lo que saque
    Stack removed(stack.get_item(), to_remove);

    // si queda vacio lo hago null, y si no le resto la cantidad a remover
    if (to_remove == current) {
        stacks[index].reset();
    } else {
        stack.set_quantity(current - to_remove);
    }

    return removed;
}

// retura cantidad determinada de objetos sin importar donde esten
// principalmente para las ventas
bool Storage::remove_quantity(const Item& item, int quantity) {
    if (quantity <= 0 || count_item(item) < quantity) {
        return false;
    }

    for (auto& slot : stacks) {
        if (quantity == 0) {
            break;
        }
        if (!slot || !(slot->get_item() == item)) {
            continue;
        }

        const int current = slot->get_quantity();
        const int to_remove = std::min(current, quantity);

        if (to_remove == current) {
            slot.reset();
        } else {
            slot->set_quantity(current - to_remove);
        }

        quantity -= to_remove;
    }

    return true;
}

// cuenta la cantidad total de un objeto en el inventario, sumando todas sus pilas
int Storage::count_item(const Item& item) const {
    int total = 0;

    for (const auto& slot : stacks) {
        if (slot && slot->get_item() == item) {
            total += slot->get_quantity();
        }
    }

    return total;
}

// permite consultar todos los casilleros del inventario, tanto ocupados como vacios
const std::vector<std::optional<Stack>>& Storage::get_stacks() const {
    return stacks;
}

bool Storage::transfer_item(Storage& destination, int index, RemoveMode mode) {
    if (this == &destination ||
        index < 0 ||
        static_cast<size_t>(index) >= stacks.size() ||
        !stacks[index]) {
        return false;
    }

    const Stack& stack = *stacks[index];

    const Item item = stack.get_item();

    const int quantity =
        quantity_to_remove(stack.get_quantity(), mode);

    if (quantity == 0 ||
        !destination.can_add_item(item, quantity)) {
        return false;
    }

    const auto removed = remove_item(index, mode);

    if (!removed) {
        return false;
    }

    if (destination.add_item(
            removed->get_item(),
            removed->get_quantity()) != 0) {
        throw std::logic_error(
            "No se pudo completar la transferencia");
    }

    return true;
}
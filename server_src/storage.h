
#ifndef STORAGE_H
#define STORAGE_H

#include <optional>
#include <vector>

#include "../common_src/item_types.h"
#include "item.h"
#include "stack.h"

class Storage {
private:
    std::vector<std::optional<Stack>> stacks;

public:
    explicit Storage(int capacity);

    int add_item(const Item& item, int quantity);
    bool can_add_item(const Item& item, int quantity) const;
    bool place_item(int index, const Item& item, int quantity);

    std::optional<Stack> remove_item(int index, RemoveMode mode);
    bool remove_quantity(const Item& item, int quantity);

    int count_item(const Item& item) const;

    const std::vector<std::optional<Stack>>& get_stacks() const;
};

#endif
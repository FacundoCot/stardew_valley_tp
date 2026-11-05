#ifndef STORAGE_H
#define STORAGE_H

#include "../common_src/item_types.h"
#include "stack.h"
#include "item.h"
#include <optional>

#include <vector>

class Storage {
    private:
        std::vector<Stack> stacks;
        int capacity;

    public:
        explicit Storage(int capacity);
        
        int add_item(const Item& item, int quantity);
        std::optional<Stack> remove_item(int index, RemoveMode mode);
        int count_item(const Item& item) const;
        const std::vector<Stack>& get_stacks() const;    
};

#endif

#ifndef STORAGE_H
#define STORAGE_H

#include "server_structs.h"
#include "stack.h"
#include "item.h"

#include <vector>

class Storage {
    private:
        std::vector<Stack> stacks;
        int capacity;

    public:
        explicit Storage(int capacity);
        
        int add_item(const Item& item, int quantity);
        bool remove_item(const Item& item, int quantity);
        int count_item(const Item& item) const;
        const std::vector<Stack>& get_stacks() const;    
};

#endif

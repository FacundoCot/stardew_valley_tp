#ifndef STACK_H
#define STACK_H

#include "item.h"
#include "item_types.h"

class Stack {
    private:
        Item item;
        int quantity;

    public:
        explicit Stack(Item item, int quantity);
        Item get_item() const;
        int get_quantity() const;
        bool set_quantity(int quantity);
};

#endif

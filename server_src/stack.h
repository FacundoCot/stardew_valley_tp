#ifndef STACK_H
#define STACK_H

#include "item.h"
#include "server_structs.h"

class Stack {
    private:
        Item item;
        unsigned int quantity;

    public:
        explicit Stack(Item item, unsigned int quantity);
        Item get_item() const;
        int get_quantity() const;
        bool set_quantity(int quantity);
};

#endif

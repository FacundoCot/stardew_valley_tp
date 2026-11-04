#ifndef ITEM_H
#define ITEM_H

#include "server_structs.h"

class Item { 
    private:
        ItemType type;
        Quality quality;

    public:
        explicit Item(ItemType type, Quality quality);
        ItemType get_type() const;
        Quality get_quality() const;
        bool is_stackable() const;
        bool operator==(const Item& other) const;
};

#endif

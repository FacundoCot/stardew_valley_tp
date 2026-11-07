#include "item.h"

Item::Item(ItemType type, Quality quality) : type(type), quality(quality) {}

ItemType Item::get_type() const {
    return type;
}

Quality Item::get_quality() const {
    return quality;
}

bool Item::is_stackable() const {
    switch (this->type) {
        case ItemType::PICKAXE:
        case ItemType::WATERING_CAN:
        case ItemType::SHOVEL:
        case ItemType::SWORD:
        case ItemType::BED:
            return false;
            break;

        default:
            return true;
            break;
    }
}

bool Item::operator==(const Item& other) const {
    return type == other.type &&
           quality == other.quality;
}
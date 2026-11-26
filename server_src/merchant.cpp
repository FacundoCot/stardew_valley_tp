
#include "merchant.h"

Merchant::Merchant(const ItemCatalog& catalog) :
        item_catalog(catalog) {}

void Merchant::add_item_for_sale(ItemType type) {
    items_for_sale.insert(type);
}

void Merchant::add_item_to_buy(ItemType type) {
    items_to_buy.insert(type);
}

bool Merchant::sells(ItemType type) const {
    return items_for_sale.count(type) > 0;
}

bool Merchant::buys(ItemType type) const {
    return items_to_buy.count(type) > 0;
}

std::optional<int> Merchant::get_buy_price(ItemType type) const {
    if (!sells(type)) {
        return std::nullopt;
    }

    return item_catalog.get_buy_price(type);
}

std::optional<int> Merchant::get_sell_price(ItemType type, Quality quality) const {

    if (!buys(type)) {
        return std::nullopt;
    }

    return item_catalog.get_sell_price(type, quality);
}

const std::set<ItemType>& Merchant::get_items_for_sale() const {
    return items_for_sale;
}

const std::set<ItemType>& Merchant::get_items_to_buy() const {
    return items_to_buy;
}

// Pierre
Pierre::Pierre(const ItemCatalog& catalog) : Merchant(catalog) {
    add_item_for_sale(ItemType::STRAWBERRY_SEED);
    add_item_for_sale(ItemType::ONION_SEED);
    add_item_for_sale(ItemType::POTATO_SEED);
    add_item_for_sale(ItemType::PUMPKIN_SEED);

    add_item_to_buy(ItemType::STRAWBERRY);
    add_item_to_buy(ItemType::ONION);
    add_item_to_buy(ItemType::POTATO);
    add_item_to_buy(ItemType::PUMPKIN);
    add_item_to_buy(ItemType::MILK);
    add_item_to_buy(ItemType::EGG);
}

// Clint
Clint::Clint(const ItemCatalog& catalog) : Merchant(catalog) {
    add_item_to_buy(ItemType::COAL);
    add_item_to_buy(ItemType::LAPIS_LAZULI);
    add_item_to_buy(ItemType::SILVER);
    add_item_to_buy(ItemType::GOLD);
}

// Robin
Robin::Robin(const ItemCatalog& catalog, int day) :
        Merchant(catalog) {

    if (day % 2 != 0) {
        add_item_for_sale(ItemType::CHEST);
        add_item_for_sale(ItemType::CHAIR);
        add_item_for_sale(ItemType::BARREL_CLOSED);
        add_item_for_sale(ItemType::CRATE_1);
        add_item_for_sale(ItemType::FLOWERS_1);
        add_item_for_sale(ItemType::PICTURE_1);
        add_item_for_sale(ItemType::WELL);
    } else {
        add_item_for_sale(ItemType::STATUE);
        add_item_for_sale(ItemType::BARREL_SWORDS);
        add_item_for_sale(ItemType::BARREL_WATER);
        add_item_for_sale(ItemType::CRATE_2);
        add_item_for_sale(ItemType::FLOWERS_2);
        add_item_for_sale(ItemType::PICTURE_2);
        add_item_for_sale(ItemType::BEAM);
    }
}

// Marnie
Marnie::Marnie(const ItemCatalog& catalog) : Merchant(catalog) {
    // La compra y venta de animales se implementara
    // cuando tengamos las estructuras correspondientes.
}

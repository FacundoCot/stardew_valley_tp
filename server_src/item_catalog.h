#ifndef ITEM_CATALOG_H
#define ITEM_CATALOG_H

#include <map>
#include <optional>
#include <string>

#include "../common_src/item_types.h"
#include "game_config.h"

struct CatalogEntry {
    std::string name;
    ItemType type;
    std::optional<int> buy_price;
    std::optional<int> base_sell_price;
};

class ItemCatalog {
private:
    std::map<ItemType, CatalogEntry> entries;

    void add_entry(ItemType type, const std::string& name, const GameConfig& config);

    bool is_crop(ItemType type) const;
    float quality_multiplier(Quality quality) const;

public:
    explicit ItemCatalog(const GameConfig& config);

    std::optional<int> get_buy_price(ItemType type) const;
    std::optional<int> get_sell_price(ItemType type, Quality quality) const;

    std::optional<ItemType> get_item_type(const std::string& name) const;
    const std::string& get_item_name(ItemType type) const;
};

#endif
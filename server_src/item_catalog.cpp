#include "item_catalog.h"

#include <cmath>
#include <stdexcept>
#include <utility>

bool ItemCatalog::is_crop(ItemType type) const {
    return type == ItemType::STRAWBERRY ||
           type == ItemType::ONION ||
           type == ItemType::POTATO ||
           type == ItemType::PUMPKIN;
}

float ItemCatalog::quality_multiplier(Quality quality) const {
    switch (quality) {
        case Quality::BASE: return 1.0;
        case Quality::GOOD: return 1.25;
        case Quality::EXCELLENT: return 2.0;
        case Quality::INCREDIBLE: return 4.0;
        case Quality::PERFECT: return 8.0;
    }

    throw std::invalid_argument("Calidad invalida");
}

void ItemCatalog::add_entry(ItemType type, const std::string& name, const GameConfig& config) {
    CatalogEntry entry{name, type, std::nullopt, std::nullopt};

    auto buy_it = config.buy_prices.find(name);

    if (buy_it != config.buy_prices.end()) {
        entry.buy_price = buy_it->second;
    }

    auto sell_it = config.sell_prices.find(name);

    if (sell_it != config.sell_prices.end()) {
        entry.base_sell_price = sell_it->second;
    }

    entries.emplace(type, entry);
}

std::optional<int> ItemCatalog::get_buy_price(ItemType type) const {
    auto it = entries.find(type);
    if (it == entries.end()) {
        return std::nullopt;
    }

    return it->second.buy_price;
}

std::optional<int> ItemCatalog::get_sell_price(ItemType type, Quality quality) const {
    auto it = entries.find(type);

    if (it == entries.end() || !it->second.base_sell_price) {
        return std::nullopt;
    }

    float base_price = *it->second.base_sell_price;

    if (!is_crop(type)) {
        return base_price;
    }

    return static_cast<int>(base_price * quality_multiplier(quality));
}

ItemCatalog::ItemCatalog(const GameConfig& config) {
    // Cultivos
    add_entry(ItemType::STRAWBERRY, "frutilla", config);
    add_entry(ItemType::ONION, "cebolla", config);
    add_entry(ItemType::POTATO, "papa", config);
    add_entry(ItemType::PUMPKIN, "calabaza", config);

    // Semillas
    add_entry(ItemType::STRAWBERRY_SEED, "semilla_frutilla", config);
    add_entry(ItemType::ONION_SEED, "semilla_cebolla", config);
    add_entry(ItemType::POTATO_SEED, "semilla_papa", config);
    add_entry(ItemType::PUMPKIN_SEED, "semilla_calabaza", config);

    // Minerales
    add_entry(ItemType::COAL, "carbon", config);
    add_entry(ItemType::LAPIS_LAZULI, "lapislazuli", config);
    add_entry(ItemType::SILVER, "plata", config);
    add_entry(ItemType::GOLD, "oro", config);

    // Productos animales
    add_entry(ItemType::MILK, "leche", config);
    add_entry(ItemType::EGG, "huevo", config);

    // Otros
    add_entry(ItemType::CHEST, "cofre", config);
}

std::optional<ItemType> ItemCatalog::get_item_type(const std::string& name) const {

    for (const auto& [type, entry] : entries) {
        if (entry.name == name) {
            return type;
        }
    }
    return std::nullopt;
}

const std::string& ItemCatalog::get_item_name(ItemType type) const {
    return entries.at(type).name;
}
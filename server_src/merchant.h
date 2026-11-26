
#ifndef MERCHANT_H
#define MERCHANT_H

#include <optional>
#include <set>

#include "../common_src/item_types.h"
#include "item_catalog.h"
#include "../common_src/merchant_pos.h"

class Merchant {
private:
    const ItemCatalog& item_catalog;

    std::set<ItemType> items_for_sale;
    std::set<ItemType> items_to_buy;

protected:
    // con el constructor protected podemos solamente crear tipos concretos,
    // como Pierre(catalog), no podemos hacer Merchant(catalog), lo cual es
    // algo que no queremos permitir hacer
    explicit Merchant(const ItemCatalog& catalog);

    // con protected los hijos pueden agregar y quitar cosas del set
    // pero no saben que es lo que hay detras, no saben como se guardan
    // las cosas digamos
    void add_item_for_sale(ItemType type);
    void add_item_to_buy(ItemType type);

public:
    // quiero tener en cuenta la clase padre a la hora de destruir los hijos
    // ya que vamos a usar merchant con polimorfismo y si no definimos el destructor
    // virtual cuando destruimos con polimorfismo hay comportamiento indefinido
    virtual ~Merchant() = default;

    bool sells(ItemType type) const;
    bool buys(ItemType type) const;

    std::optional<int> get_buy_price(ItemType type) const;
    std::optional<int> get_sell_price(ItemType type, Quality quality) const;

    const std::set<ItemType>& get_items_for_sale() const;
    const std::set<ItemType>& get_items_to_buy() const;
};

class Pierre : public Merchant {
public:
    explicit Pierre(const ItemCatalog& catalog);
};

class Clint : public Merchant {
public:
    explicit Clint(const ItemCatalog& catalog);
};

class Robin : public Merchant {
public:
    Robin(const ItemCatalog& catalog, int day);
};

class Marnie : public Merchant {
public:
    explicit Marnie(const ItemCatalog& catalog);
};

#endif

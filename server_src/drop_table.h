#ifndef DROP_TABLE_H
#define DROP_TABLE_H

#include <utility>
#include <vector>

#include "enemy_config.h"
#include "enemy.h"
#include "item.h"
#include "random.h"

class DropTable {
private:
    const EnemyConfig& config;

public:
    explicit DropTable(const EnemyConfig& config): config(config) {}
    std::vector<std::pair<Item, int>> roll(EnemyKind kind, Random& rng) const;
    std::vector<std::pair<Item, int>> roll(const Enemy& enemy, Random& rng) const;
};

#endif

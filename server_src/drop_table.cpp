#include "drop_table.h"

#include "vampire.h"

namespace {
std::vector<std::pair<Item, int>> roll_rules(
    const std::vector<EnemyDropRule>& rules, Random& rng) {
    std::vector<std::pair<Item, int>> result;
    for (const EnemyDropRule& rule : rules) {
        if (rng.next01() < rule.probability) {
            result.emplace_back(Item(rule.item_type, rule.quality),
                                rng.range(rule.min_quantity, rule.max_quantity));
        }
    }
    return result;
}
}  // namespace

std::vector<std::pair<Item, int>> DropTable::roll(EnemyKind kind, Random& rng) const {
    switch (kind) {
        case EnemyKind::SKELETON:
            return roll_rules(config.skeleton.drops, rng);
        case EnemyKind::ORC:
            return roll_rules(config.orc.drops, rng);
        default:
            return {};
    }
}

std::vector<std::pair<Item, int>> DropTable::roll(
    const Enemy& enemy, Random& rng) const {
    if (enemy.kind() == EnemyKind::VAMPIRE) {
        const auto* vampire = dynamic_cast<const Vampire*>(&enemy);
        if (vampire == nullptr) {
            return {};
        }
        const auto& rules = vampire->is_female() ? config.female_vampire.drops
                                                 : config.male_vampire.drops;
        return roll_rules(rules, rng);
    }
    return roll(enemy.kind(), rng);
}

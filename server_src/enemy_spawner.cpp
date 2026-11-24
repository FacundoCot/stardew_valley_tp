#include "enemy_spawner.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <set>

#include "../common_src/cave_level.h"
#include "orc.h"
#include "skeleton.h"
#include "skull.h"
#include "vampire.h"

namespace {
EnemyStats stats(const EnemyTypeConfig& c) {
    return {c.max_health, c.damage, c.speed, c.attack_cooldown, c.attack_range};
}
bool near(const std::pair<float, float>& a, const std::pair<float, float>& b) {
    return std::max(std::abs(a.first - b.first), std::abs(a.second - b.second)) <= 2.5f;
}
}

std::vector<std::unique_ptr<Enemy>> EnemySpawner::populate(
    ZoneId zone, const CaveLevel& level, const World&) {
    const auto& points = level.enemy_spawns();
    std::vector<int> free(points.size());
    for (size_t i = 0; i < points.size(); ++i) free[i] = static_cast<int>(i);
    std::vector<std::vector<int>> groups;
    std::set<int> seen;
    for (int index : free) {
        if (seen.count(index)) continue;
        std::vector<int> group{index};
        seen.insert(index);
        for (size_t cursor = 0; cursor < group.size(); ++cursor) {
            for (int candidate : free) {
                if (!seen.count(candidate) && near(points[group[cursor]], points[candidate])) {
                    seen.insert(candidate); group.push_back(candidate);
                }
            }
        }
        groups.push_back(std::move(group));
    }
    std::vector<std::unique_ptr<Enemy>> result;
    int next_group = 1;
    auto take = [&](int index) {
        free.erase(std::remove(free.begin(), free.end(), index), free.end());
    };
    auto make_orc = [&](int index, int group) {
        auto enemy = std::make_unique<Orc>(zone, points[index].first, points[index].second, stats(config.orc));
        enemy->set_group_id(group);
        result.push_back(std::move(enemy));
    };
    std::vector<int> order(groups.size());
    for (size_t i = 0; i < groups.size(); ++i) order[i] = static_cast<int>(i);
    rng.shuffle(order.begin(), order.end());
    const int count = rng.range(config.orc_group_min, config.orc_group_max);
    bool orcs_placed = false;
    for (int group_index : order) {
        const auto& group = groups[group_index];
        if (static_cast<int>(group.size()) < count) continue;
        std::vector<int> selected = group;
        rng.shuffle(selected.begin(), selected.end());
        for (int i = 0; i < count; ++i) {
            make_orc(selected[i], next_group);
            take(selected[i]);
        }
        ++next_group;
        orcs_placed = true;
        break;
    }
    if (!orcs_placed) {
        std::cerr << "No hay espacio para orcos en zona " << zone.level << "\n";
    }
    const int pair_count = rng.range(config.vampire_pairs_min, config.vampire_pairs_max);
    for (int pair = 0; pair < pair_count; ++pair) {
        bool placed = false;
        std::vector<std::pair<int, int>> candidates;
        for (size_t i = 0; i < points.size(); ++i) {
            if (std::find(free.begin(), free.end(), static_cast<int>(i)) == free.end()) continue;
            for (size_t j = i + 1; j < points.size(); ++j) {
                if (std::find(free.begin(), free.end(), static_cast<int>(j)) != free.end() &&
                    near(points[i], points[j])) {
                    candidates.emplace_back(static_cast<int>(i), static_cast<int>(j));
                }
            }
        }
        rng.shuffle(candidates.begin(), candidates.end());
        if (!candidates.empty()) {
            const auto [first, second] = candidates.front();
                auto female = std::make_unique<Vampire>(zone, points[first].first, points[first].second,
                                                         stats(config.female_vampire), true);
                auto male = std::make_unique<Vampire>(zone, points[second].first, points[second].second,
                                                       stats(config.male_vampire), false);
                female->set_frenzy_multiplier(config.frenzy_multiplier);
                male->set_frenzy_multiplier(config.frenzy_multiplier);
                female->set_group_id(next_group); male->set_group_id(next_group);
                result.push_back(std::move(female)); result.push_back(std::move(male));
                take(first); take(second); ++next_group;
                placed = true;
        }
        if (!placed) std::cerr << "No hay espacio para vampiros en zona " << zone.level << "\n";
    }
    const int skeleton_count = rng.range(config.skeletons_min, config.skeletons_max);
    for (int i = 0; i < skeleton_count; ++i) {
        if (free.empty()) { std::cerr << "No hay espacio para esqueletos en zona " << zone.level << "\n"; break; }
        const int at = rng.range(0, static_cast<int>(free.size()) - 1);
        const int point = free[at];
        result.push_back(std::make_unique<Skeleton>(zone, points[point].first, points[point].second,
                                                    stats(config.skeleton), config.skeleton_revive_seconds));
        take(point);
    }
    for (int i = 0; i < config.skulls_per_level; ++i) {
        if (free.empty()) { std::cerr << "No hay espacio para calaveras en zona " << zone.level << "\n"; break; }
        const int point = free[rng.range(0, static_cast<int>(free.size()) - 1)];
        result.push_back(std::make_unique<Skull>(zone, points[point].first, points[point].second,
                                                 rng.range(0, 1) == 1, config.skull));
        take(point);
    }
    return result;
}

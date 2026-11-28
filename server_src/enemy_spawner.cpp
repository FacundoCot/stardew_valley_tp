#include "enemy_spawner.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>
#include <vector>

#include "../common_src/cave_level.h"
#include "orc.h"
#include "skeleton.h"
#include "skull.h"
#include "vampire.h"

namespace {
enum class SpawnCategory {
    ORC,
    VAMPIRE_PAIR,
    SKELETON,
    SKULL
};

EnemyStats stats(const EnemyTypeConfig& c) {
    return {c.max_health, c.damage, c.speed, c.attack_cooldown, c.attack_range};
}

std::vector<std::pair<float, float>> group_positions(
        const CaveLevel& level, const std::pair<float, float>& anchor,
        const std::vector<std::pair<float, float>>& spawn_points,
        int count, Random& rng) {
    constexpr int SPAWN_SEARCH_RADIUS = 2;
    const int anchor_x = static_cast<int>(std::floor(anchor.first));
    const int anchor_y = static_cast<int>(std::floor(anchor.second));
    std::vector<std::pair<float, float>> candidates;
    for (int y = anchor_y - SPAWN_SEARCH_RADIUS;
         y <= anchor_y + SPAWN_SEARCH_RADIUS; ++y) {
        for (int x = anchor_x - SPAWN_SEARCH_RADIUS;
             x <= anchor_x + SPAWN_SEARCH_RADIUS; ++x) {
            const std::pair<float, float> position{
                static_cast<float>(x) + 0.5f,
                static_cast<float>(y) + 0.5f
            };
            bool is_other_spawn = false;
            for (const auto& spawn : spawn_points) {
                if (static_cast<int>(std::floor(spawn.first)) == x &&
                    static_cast<int>(std::floor(spawn.second)) == y &&
                    (x != anchor_x || y != anchor_y)) {
                    is_other_spawn = true;
                    break;
                }
            }
            if (!is_other_spawn &&
                level.is_walkable(position.first, position.second)) {
                candidates.push_back(position);
            }
        }
    }
    rng.shuffle(candidates.begin(), candidates.end());

    std::vector<std::pair<float, float>> positions;
    positions.reserve(static_cast<size_t>(count));
    for (int i = 0; i < count && i < static_cast<int>(candidates.size()); ++i) {
        positions.push_back(candidates[static_cast<size_t>(i)]);
    }
    while (static_cast<int>(positions.size()) < count) {
        positions.push_back(anchor);
    }
    return positions;
}
}

std::vector<std::unique_ptr<Enemy>> EnemySpawner::populate(
    ZoneId zone, const CaveLevel& level, const World&) {
    if (config.skulls_per_level > 1) {
        throw std::invalid_argument(
            "spawn.calaveras_por_nivel no puede superar 1");
    }
    const auto& points = level.enemy_spawns();
    std::vector<SpawnCategory> categories{
        SpawnCategory::ORC
    };
    const int vampire_pairs = rng.range(config.vampire_pairs_min,
                                        config.vampire_pairs_max);
    for (int i = 0; i < vampire_pairs; ++i) {
        categories.push_back(SpawnCategory::VAMPIRE_PAIR);
    }
    const int skeleton_count = rng.range(config.skeletons_min,
                                         config.skeletons_max);
    const size_t skeleton_sites =
        std::min(static_cast<size_t>(skeleton_count), points.size());
    for (size_t i = 0; i < skeleton_sites; ++i) {
        categories.push_back(SpawnCategory::SKELETON);
    }
    if (config.skulls_per_level == 1) {
        categories.push_back(SpawnCategory::SKULL);
    }

    rng.shuffle(categories.begin(), categories.end());
    if (categories.size() > points.size()) {
        categories.resize(points.size());
    }
    while (categories.size() < points.size()) {
        categories.push_back(SpawnCategory::ORC);
    }
    rng.shuffle(categories.begin(), categories.end());

    std::vector<size_t> point_order(points.size());
    for (size_t i = 0; i < point_order.size(); ++i) {
        point_order[i] = i;
    }
    rng.shuffle(point_order.begin(), point_order.end());

    std::vector<std::unique_ptr<Enemy>> result;
    int next_group = 1;
    for (size_t i = 0; i < point_order.size(); ++i) {
        const auto& anchor = points[point_order[i]];
        switch (categories[i]) {
            case SpawnCategory::ORC: {
                const int count = rng.range(config.orc_group_min,
                                            config.orc_group_max);
                const auto positions = group_positions(
                    level, anchor, points, count, rng);
                for (const auto& position : positions) {
                    auto enemy = std::make_unique<Orc>(
                        zone, position.first, position.second, stats(config.orc));
                    enemy->set_group_id(next_group);
                    result.push_back(std::move(enemy));
                }
                ++next_group;
                break;
            }
            case SpawnCategory::VAMPIRE_PAIR: {
                const auto positions = group_positions(
                    level, anchor, points, 2, rng);
                auto female = std::make_unique<Vampire>(
                    zone, positions[0].first, positions[0].second,
                    stats(config.female_vampire), true);
                auto male = std::make_unique<Vampire>(
                    zone, positions[1].first, positions[1].second,
                    stats(config.male_vampire), false);
                female->set_frenzy_multiplier(config.frenzy_multiplier);
                male->set_frenzy_multiplier(config.frenzy_multiplier);
                female->set_group_id(next_group);
                male->set_group_id(next_group);
                result.push_back(std::move(female));
                result.push_back(std::move(male));
                ++next_group;
                break;
            }
            case SpawnCategory::SKELETON:
                result.push_back(std::make_unique<Skeleton>(
                    zone, anchor.first, anchor.second, stats(config.skeleton),
                    config.skeleton_revive_seconds));
                break;
            case SpawnCategory::SKULL:
                result.push_back(std::make_unique<Skull>(
                    zone, anchor.first, anchor.second, rng.range(0, 1) == 1,
                    config.skull));
                break;
        }
    }
    return result;
}

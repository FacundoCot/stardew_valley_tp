#ifndef ENEMY_SPAWNER_H
#define ENEMY_SPAWNER_H

#include <memory>
#include <vector>

#include "enemy_config.h"
#include "enemy.h"
#include "random.h"

class CaveLevel;
class World;

class EnemySpawner {
private:
    Random& rng;
    const EnemyConfig& config;

public:
    EnemySpawner(Random& rng, const EnemyConfig& config): rng(rng), config(config) {}
    std::vector<std::unique_ptr<Enemy>> populate(ZoneId zone, const CaveLevel& level,
                                                  const World& world);
};

#endif

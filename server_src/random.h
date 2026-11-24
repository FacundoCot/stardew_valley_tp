#ifndef RANDOM_H
#define RANDOM_H

#include <algorithm>
#include <cstdint>
#include <random>

class Random {
private:
    std::mt19937 generator;

public:
    explicit Random(uint32_t seed): generator(seed) {}

    float next01() {
        return std::uniform_real_distribution<float>(0.0f, 1.0f)(generator);
    }

    int range(int min, int max) {
        return std::uniform_int_distribution<int>(min, max)(generator);
    }

    template <typename Iterator>
    void shuffle(Iterator first, Iterator last) {
        std::shuffle(first, last, generator);
    }
};

#endif

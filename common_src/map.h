#ifndef MAP_H
#define MAP_H

#include <string>
#include <vector>
#include <utility>


class Map {
private:
    int width;   // x
    int height;  // y
    
    std::vector<std::string> grid;  // grid[y][x] -> # pared, . piso

public:
    Map(int width, int height);

    bool is_walkable(float x, float y) const;
    const std::vector<std::string>& rows() const;
    std::vector<std::pair<float, float>> spawns() const;
};

#endif
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

    bool is_walkable(int x, int y) const;
    const std::vector<std::string>& rows() const;
    std::vector<std::pair<int, int>> spawns() const;
};

#endif
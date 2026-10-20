#include "map.h"

namespace {
const char WALL = '#';
const char FLOOR = '.';
const char SPAWN = 'S';
}  // namespace

Map::Map(int map_width, int map_height):
        width(map_width),
        height(map_height) {

    const int spawn_col = width / 2;
    const int spawn_row = height /2;


    for (int row = 0; row < height; row++) {
        std::string line;
        for (int col = 0; col < width; col++) {
            if (col == 0 || row == 0 || col == width - 1 || row == height - 1)
            {
                line += WALL;
            }
            else if (col == spawn_col && row == spawn_row){
                line += SPAWN;
            }
            else{
                line += FLOOR;
            }
        }
        grid.push_back(line);
    }
}

bool Map::is_walkable(int x, int y) const {
    if (x < 0 || y < 0 || x >= width || y >= height) {
        return false;
    }
    return grid[y][x] != WALL;
}

const std::vector<std::string>& Map::rows() const {
    return grid;
}

std::vector<std::pair<int, int>> Map::spawns() const {
    std::vector<std::pair<int, int>> result;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (grid[y][x] == SPAWN) {
                result.emplace_back(x, y);
            }
        }
    }
    return result;
}
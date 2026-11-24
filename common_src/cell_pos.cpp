#include "cell_pos.h"

bool CellPos::operator<(const CellPos& other) const {
    if (x != other.x) {
        return x < other.x;
    }

    return y < other.y;
}
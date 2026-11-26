#include "chest.h"

bool Chest::is_opened() const {
    return opened_by != -1;
}

int Chest::get_opened_by() const {
    return opened_by;
}

bool Chest::open(int player_id) {
    if (is_opened() || player_id < 0) {
        return false;
    }

    opened_by = player_id;
    return true;
}

bool Chest::close(int player_id) {
    if (!is_opened() || opened_by != player_id) {
        return false;
    }

    opened_by = -1;
    return true;
}
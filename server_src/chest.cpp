#include "chest.h"

bool Chest::is_opened() const {
    return opened_by != -1;
}

void Chest::open(int player_id) {
    if (opened_by == -1) {
        opened_by = player_id;
    }
}

void Chest::close(int player_id) {
    if (opened_by == player_id) {
        opened_by = -1;
    }
}
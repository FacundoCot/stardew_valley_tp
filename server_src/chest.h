#ifndef CHEST_H
#define CHEST_H

#include "storage.h"

class Chest {
private:
    int opened_by = -1;

public:
    Storage storage{30};

    bool is_opened() const;
    int get_opened_by() const;

    bool open(int player_id);
    bool close(int player_id);
};

#endif
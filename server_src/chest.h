#ifndef CHEST_H
#define CHEST_H

#include "storage.h"

class Chest {
    private:
        int opened_by = -1;  // ID del jugador que abrió el cofre, -1 si está cerrado.
    public:
        Storage storage {30};  // Capacidad del cofre, por ejemplo, 30 espacios.
        bool is_opened() const;
        void open(int player_id);
        void close(int player_id);
};

#endif
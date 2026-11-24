#ifndef FIGHT_SET_TRACKER_H
#define FIGHT_SET_TRACKER_H

#include <map>
#include <set>
#include <string>

#include "enemy.h"

class FightSetTracker {
private:
    struct Presence {
        ZoneId room;
        bool has_skull;
        bool hurt;
    };

    std::map<std::string, Presence> presence;   // jugador -> sala actual
    std::set<ZoneId> clean_skull_rooms;         // salas con calavera cumplidas
    std::set<int> skeleton_levels;              // niveles con esqueleto vencido
    std::map<int, std::set<bool>> vampire_sexes; // group_id -> sexos vencidos (true = vampira)
    std::map<int, int> orc_groups;              // group_id -> muertes

public:
    void on_enemy_killed(const Enemy& enemy, int level);
    void on_player_entered_room(const std::string& player, ZoneId room, bool has_skull);
    void on_player_hurt_by_enemy(const std::string& player);
    void on_player_left_room(const std::string& player);

    bool skeleton_set_done() const;
    bool vampire_set_done() const;
    bool orc_set_done() const;
    bool skull_set_done() const;
};

#endif
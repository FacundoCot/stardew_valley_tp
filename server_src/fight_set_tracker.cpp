#include "fight_set_tracker.h"

#include "vampire.h"

void FightSetTracker::on_enemy_killed(const Enemy& enemy, int level) {
    if (enemy.kind() == EnemyKind::SKELETON) {
        skeleton_levels.insert(level);
    }
    if (enemy.kind() == EnemyKind::VAMPIRE) {
        const auto* vampire = dynamic_cast<const Vampire*>(&enemy);
        if (vampire != nullptr) {
            vampire_sexes[enemy.get_group_id()].insert(vampire->is_female());
        }
    }
    if (enemy.kind() == EnemyKind::ORC) {
        ++orc_groups[enemy.get_group_id()];
    }
}

void FightSetTracker::on_player_entered_room(const std::string& player, ZoneId room,
                                             bool has_skull) {
    presence[player] = Presence{room, has_skull, false};
}

void FightSetTracker::on_player_hurt_by_enemy(const std::string& player) {
    auto it = presence.find(player);
    if (it != presence.end() && it->second.has_skull) {
        it->second.hurt = true;
    }
}

void FightSetTracker::on_player_left_room(const std::string& player) {
    auto it = presence.find(player);
    if (it == presence.end()) {
        return;
    }
    if (it->second.has_skull && !it->second.hurt) {
        clean_skull_rooms.insert(it->second.room);
    }
    presence.erase(it);
}

bool FightSetTracker::skeleton_set_done() const {
    for (int level : skeleton_levels) {
        if (skeleton_levels.count(level + 1) && skeleton_levels.count(level + 2)) {
            return true;
        }
    }
    return false;
}

bool FightSetTracker::vampire_set_done() const {
    // Se necesitan la vampira y el vampiro de la misma pareja, cada uno vencido una vez.
    for (const auto& [group, sexes] : vampire_sexes) {
        if (sexes.size() == 2) {
            return true;
        }
    }
    return false;
}

bool FightSetTracker::orc_set_done() const {
    for (const auto& [group, count] : orc_groups) {
        if (count >= 4) {
            return true;
        }
    }
    return false;
}

bool FightSetTracker::skull_set_done() const {
    return !clean_skull_rooms.empty();
}
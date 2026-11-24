#ifndef CROP_H
#define CROP_H

#include "../common_src/item_types.h"
#include "save_data_structs.h"
#include <optional>

const int DAY_MINUTES = 1200;

class Crop {
private:
    ItemType type;

    int required_waterings;
    int growth_days;
    int regrowth_days;
    int tolerated_missed_waterings;

    int days_grown = 0;
    int waterings_today = 0;
    int last_watering_window = -1;
    int missed_waterings = 0;
    int watering_delay_minutes = 0;
    int ready_days = 0;

    bool ready_to_harvest = false;

public:
    Crop(ItemType type, int required_waterings,
         int growth_days, int regrowth_days, int tolerated_miss_water);

    ItemType get_type() const;
    bool is_ready_to_harvest() const;

    bool needs_water(int elapsed_minutes) const;
    bool water(int elapsed_minutes);

    void advance_day();
    bool start_regrowth();

    int quality_score(
    int elapsed_minutes,
    int luck_percent,
    int watering_minutes_per_point,
    int missed_watering_penalty,
    int harvest_minutes_per_point) const;

    CropSaveData get_save_data() const;
    static Crop from_save_data(const CropSaveData& data);

    int get_days_grown() const;
    int get_growth_days() const;
};

#endif
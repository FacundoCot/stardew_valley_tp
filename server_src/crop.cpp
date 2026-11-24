
#include "crop.h"

#include <stdexcept>
#include <algorithm>

Crop::Crop(ItemType type, int required_waterings,
           int growth_days, int regrowth_days,
           int tolerated_missed_waterings):
        type(type),
        required_waterings(required_waterings),
        growth_days(growth_days),
        regrowth_days(regrowth_days),
        tolerated_missed_waterings(tolerated_missed_waterings) {

    if (required_waterings <= 0 ||
        growth_days <= 0 ||
        regrowth_days < 0 ||
        tolerated_missed_waterings < 0 ||
        tolerated_missed_waterings >= required_waterings) {
        throw std::invalid_argument("Parametros de cultivo invalidos");
    }
}

ItemType Crop::get_type() const {
    return type;
}

bool Crop::is_ready_to_harvest() const {
    return ready_to_harvest;
}

bool Crop::needs_water(int elapsed_minutes) const {
    if (ready_to_harvest ||
        elapsed_minutes < 0 ||
        elapsed_minutes >= DAY_MINUTES) {
        return false;
    }

    const int window =
        elapsed_minutes * required_waterings / DAY_MINUTES;

    return window != last_watering_window &&
           waterings_today < required_waterings;
}

bool Crop::water(int elapsed_minutes) {
    if (!needs_water(elapsed_minutes)) {
        return false;
    }

    const int window =
        elapsed_minutes * required_waterings / DAY_MINUTES;

    const int window_start =
        window * DAY_MINUTES / required_waterings;

    watering_delay_minutes += elapsed_minutes - window_start;

    last_watering_window = window;
    ++waterings_today;

    return true;
}

void Crop::advance_day() {
    if (!ready_to_harvest) {
        const int missing = required_waterings - waterings_today;

        missed_waterings += missing;

        if (missing <= tolerated_missed_waterings) {
            ++days_grown;

            if (days_grown >= growth_days) {
                ready_to_harvest = true;
            }
        }
    } else {
        ++ready_days; 
    }

    waterings_today = 0;
    last_watering_window = -1;
}

bool Crop::start_regrowth() {
    if (!ready_to_harvest || regrowth_days <= 0) {
        return false;
    }

    growth_days = regrowth_days;
    days_grown = 0;

    waterings_today = 0;
    last_watering_window = -1;
    missed_waterings = 0;
    watering_delay_minutes = 0;
    ready_days = 0;

    ready_to_harvest = false;

    return true;
}

int Crop::quality_score(
    int elapsed_minutes,
    int luck_percent,
    int watering_minutes_per_point,
    int missed_watering_penalty,
    int harvest_minutes_per_point) const {

    if (!ready_to_harvest) {
        return 0;
    }

    const int harvest_delay = ready_days * DAY_MINUTES + elapsed_minutes;

    const int watering_penalty = watering_delay_minutes / watering_minutes_per_point;

    const int missing_penalty = missed_waterings * missed_watering_penalty;

    const int harvest_penalty = harvest_delay / harvest_minutes_per_point;

    const int total_penalty = watering_penalty + missing_penalty + harvest_penalty;

    const int base_score = std::clamp(100 - total_penalty, 0, 100);

    return std::clamp(base_score * luck_percent / 100, 0, 100);
}

Crop Crop::from_save_data(const CropSaveData& data) {
    Crop crop(
        data.type,
        data.required_waterings,
        data.growth_days,
        data.regrowth_days,
        data.tolerated_missed_waterings
    );

    crop.days_grown = data.days_grown;
    crop.waterings_today = data.waterings_today;
    crop.last_watering_window = data.last_watering_window;
    crop.missed_waterings = data.missed_waterings;
    crop.watering_delay_minutes = data.watering_delay_minutes;
    crop.ready_days = data.ready_days;
    crop.ready_to_harvest = data.ready_to_harvest;

    return crop;
}

CropSaveData Crop::get_save_data() const {
    CropSaveData data{};

    data.type = type;
    data.required_waterings = required_waterings;
    data.growth_days = growth_days;
    data.regrowth_days = regrowth_days;
    data.tolerated_missed_waterings = tolerated_missed_waterings;

    data.days_grown = days_grown;
    data.waterings_today = waterings_today;
    data.last_watering_window = last_watering_window;
    data.missed_waterings = missed_waterings;
    data.watering_delay_minutes = watering_delay_minutes;
    data.ready_days = ready_days;
    data.ready_to_harvest = ready_to_harvest;

    return data;
}

int Crop::get_days_grown() const {
    return days_grown;
}

int Crop::get_growth_days() const {
    return growth_days;
}

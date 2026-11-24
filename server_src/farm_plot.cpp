#include "farm_plot.h"

FarmPlot::FarmPlot() = default;

bool FarmPlot::has_crop() const {
    return crop.has_value();
}

bool FarmPlot::plant(ItemType type, int required_waterings,
                     int growth_days, int regrowth_days, 
                     int tolerated_missed_waterings) {
    if (has_crop()) {
        return false;
    }

    crop.emplace(type, required_waterings, growth_days, 
                regrowth_days, tolerated_missed_waterings);

    return true;
}

const Crop* FarmPlot::get_crop() const {
    return crop ? &crop.value() : nullptr;
}

bool FarmPlot::needs_water(int elapsed_minutes) const {
    return crop && crop->needs_water(elapsed_minutes);
}

bool FarmPlot::water(int elapsed_minutes) {
    return crop && crop->water(elapsed_minutes);
}

void FarmPlot::advance_day() {
    if (crop) {
        crop->advance_day();
    }
}

std::optional<ItemType> FarmPlot::harvest() {
    if (!crop || !crop->is_ready_to_harvest()) {
        return std::nullopt;
    }

    const ItemType type = crop->get_type();

    if (!crop->start_regrowth()) {
        crop.reset();
    }

    return type;
}

void FarmPlot::restore_crop(const CropSaveData& data) {
    crop = Crop::from_save_data(data);
}
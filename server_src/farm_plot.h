#ifndef FARM_PLOT_H
#define FARM_PLOT_H

#include <optional>

#include "crop.h"

class FarmPlot {
private:
    std::optional<Crop> crop;

public:
    FarmPlot();

    bool has_crop() const;

    bool plant(ItemType type, int required_waterings,
               int growth_days, int regrowth_days,
               int tolerated_missed_waterings);

    const Crop* get_crop() const;

    bool needs_water(int elapsed_minutes) const;
    bool water(int elapsed_minutes);
    std::optional<ItemType> harvest();
    void advance_day();
    
    //guardado y restauracion
    void restore_crop(const CropSaveData& data);
};

#endif
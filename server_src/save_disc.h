
#ifndef SAVE_DISC_H
#define SAVE_DISC_H

#include <filesystem>
#include <string>
#include <vector>

#include "save_data_structs.h"

class SaveDisc {
private:
    std::filesystem::path saves_directory;

public:
    explicit SaveDisc(const std::filesystem::path& saves_directory);

    void save(const GameSaveData& data) const;
    GameSaveData load(const std::string& game_code) const;
    std::vector<std::string> list_saves() const;
};

#endif

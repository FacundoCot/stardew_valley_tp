#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <SDL2pp/Mixer.hh>
#include <SDL2pp/Chunk.hh>
#include <SDL2pp/Music.hh>
#include <unordered_map>
#include <string>

#include "../common_src/zone.h"

class AudioManager {

private:
SDL2pp::Mixer mixer;
std::unordered_map<std::string, SDL2pp::Chunk> sfx;
std::unordered_map<std::string, SDL2pp::Music> music;
std::string current_music;

std::unordered_map<ZoneId::Kind, std::string> audio_zone = {
    {ZoneId::Kind::FARM, "farm"},
    {ZoneId::Kind::VILLAGE, "village"},
    {ZoneId::Kind::CAVE_LEVEL, "cave_level"}
};


public:
AudioManager();

void load_sfx(const std::string& id, const std::string& path);
void load_music(const std::string& id, const std::string& path);

void play_sfx(const std::string& id);
void play_music(ZoneId::Kind& zone, int fade_ms = 500);

void stop_music();

void set_volume_sfx(int& volume);
void set_volume_music(int& volume);

};


#endif 
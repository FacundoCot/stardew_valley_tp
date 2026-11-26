#include "audio_manager.h"
#include <iostream>

#include "../common_src/custom_error.h"


AudioManager::AudioManager() : mixer(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 2048) {}


void AudioManager::load_sfx(const std::string& id, const std::string& path) {
    try {
        sfx.emplace(id, SDL2pp::Chunk(path));

    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
    }
}

void AudioManager::load_music(const std::string& id, const std::string& path) {
    try {
        music.emplace(id, SDL2pp::Music(path));
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
    }
}

void AudioManager::play_sfx(const std::string& id) {
    auto sfx_to_play = sfx.find(id);
    if (sfx_to_play == sfx.end()) {
        throw AssetNotFound("error at AudioManager, cannot find sfx with id: " + id);
    }
    mixer.PlayChannel(-1, sfx_to_play->second, 0);
}

void AudioManager::play_music(ZoneId::Kind& id, int fade_ms) {
    std::string music_name = audio_zone.find(id)->second;

    if (current_music == music_name) {return;}

    auto music_to_play = music.find(music_name);

    if (music_to_play == music.end()) {
        throw AssetNotFound("error at AudioManager, cannot find music with id: " + music_name);
    }

    current_music = music_name;
    if (fade_ms > 0) {
        mixer.FadeInMusic(music_to_play->second, -1);
    } else {
        mixer.PlayMusic(music_to_play->second, -1);
    }
}

void AudioManager::stop_music() {
    mixer.HaltMusic();
    current_music.clear();
}

void AudioManager::set_volume_music(int& volume) {
    mixer.SetMusicVolume(volume);
}

void AudioManager::set_volume_sfx(int& volume) {
    mixer.SetVolume(-1, volume);
}
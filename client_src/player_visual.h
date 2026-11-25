#ifndef PLAYER_VISUAL_H
#define PLAYER_VISUAL_H

#include <SDL2pp/Renderer.hh>
#include <utility>
#include <unordered_map>

#include "../common_src/messages.h"
#include "camera.h"
#include "loader.h"
#include "appearance_translator.h"


struct PlayerAnim {
    float x;
    float y;
    int last_move_it = -100;

};

class PlayerVisual {
private:
SDL2pp::Renderer& renderer;
Loader& loader;

std::unordered_map<std::string, PlayerAnim> last_pos;

SDL2pp::Rect to_SDLRect(Translator::Rect& rect);
void draw_hair(AppearanceTranslator& translator, SDL2pp::Rect& body, CharacterAppearance& ap, int& player_dy, const Direction& facing);
void draw_shirt(AppearanceTranslator& translator, SDL2pp::Rect& body, CharacterAppearance& ap, int& player_dy, const Direction& facing);
void draw_pants(AppearanceTranslator& translator, SDL2pp::Rect& body, CharacterAppearance& ap, int& step_dx, const Direction& facing);
void draw_hat(AppearanceTranslator& translator, SDL2pp::Rect& body, CharacterAppearance& ap, int& player_dy, const Direction& facing);

const int COUNT_ANIMATIONS = 4;
const int STEPS[4] = {
    1,
    0,
    2,
    0
};
const int PLAYER_DY[4] = {
    1,
    0,
    1,
    0
};
const std::unordered_map<Direction, int> PLAYER_FACING = {
    {Direction::DOWN, 0},
    {Direction::RIGHT, 1},
    {Direction::LEFT, 1},
    {Direction::UP, 2}
};
const std::unordered_map<Direction, int> SHIRT_FACING = {
    {Direction::DOWN, 0},
    {Direction::RIGHT, 1},
    {Direction::LEFT, 2},
    {Direction::UP, 3}
};
const std::unordered_map<Direction, int> PANTS_FACING = {
    {Direction::DOWN, 0},
    {Direction::RIGHT, 1},
    {Direction::LEFT, 1},
    {Direction::UP, 2}
};
const std::unordered_map<Direction, int> HAIR_FACING = {
    {Direction::DOWN, 0},
    {Direction::RIGHT, 1},
    {Direction::UP, 2},
    {Direction::LEFT, 3}
};
const std::unordered_map<Direction, int> HAT_FACING = {
    {Direction::DOWN, 0},
    {Direction::RIGHT, 1},
    {Direction::UP, 3},
    {Direction::LEFT, 2}
};

const uint8_t scale = 2; 

public:
PlayerVisual(SDL2pp::Renderer& renderer, Loader& loader);

void draw( const PlayerState& state, Camera& camera, int& it);

};


#endif
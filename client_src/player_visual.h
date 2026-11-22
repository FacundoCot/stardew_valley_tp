#ifndef PLAYER_VISUAL_H
#define PLAYER_VISUAL_H

#include <SDL2pp/Renderer.hh>
#include <utility>
#include <map>

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

SDL2pp::Rect to_SDLRect(Rect& rect);
void draw_hair(AppearanceTranslator& translator, SDL2pp::Rect& body, CharacterAppearance& ap);
void draw_shirt(AppearanceTranslator& translator, SDL2pp::Rect& body, CharacterAppearance& ap);
void draw_pants(AppearanceTranslator& translator, SDL2pp::Rect& body, CharacterAppearance& ap, int& step_dx);
void draw_hat(AppearanceTranslator& translator, SDL2pp::Rect& body, CharacterAppearance& ap);

const int COUNT_ANIMATIONS = 4;
const int STEPS[4] = {
    1,
    2,
    1,
    3
};
const int PLAYER_DY[4] = {
    1,
    0,
    1,
    0
};
const uint8_t scale = 2; 

public:
PlayerVisual(SDL2pp::Renderer& renderer, Loader& loader);

void draw( const PlayerState& state, Camera& camera, int& it);

};


#endif
#ifndef CHARACTER_APPEARANCE_H
#define CHARACTER_APPEARANCE_H

#include <cstdint>

struct CharacterAppearance {
    uint16_t body = 0;
    uint16_t hair = 0;
    uint16_t shirt = 0;
    uint16_t pants = 0;
    uint16_t hat = 0;
    uint16_t hair_color = 0;
    uint16_t pants_color = 0;
};

#endif
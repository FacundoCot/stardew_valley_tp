#ifndef APPEARANCE_TRANSLATOR_H
#define APPEARANCE_TRANSLATOR_H

#include <stdint.h>
#include "../common_src/character_appearance.h"

const int BODY_COUNT = 2;// 0 = masculino, 1 = femenino
const int HAIR_COUNT = 80;
const int SHIRT_COUNT = 128;
const int PANTS_COUNT = 11;
const int HAT_COUNT = 123;   
const int COLOR_COUNT = 8;

const int BODY_WIDTH = 16;
const int BODY_HEIGHT = 32;
const int PANTS_WIDTH = 16;
const int PANTS_HEIGHT = 32;
const int HAIR_WIDTH = 16;
const int HAIR_HEIGHT = 32;
const int SHIRT_WIDTH = 8;
const int SHIRT_HEIGHT = 8;
const int HAT_WIDTH = 20;
const int HAT_HEIGHT = 20;

const int BODY_BLOCK_W = 288;
const int PANTS_BLOCK_W = 192;

const int BODY_ROW = 18;
const int PANTS_ROW = 0;

const int HAIR_PER_ROW = 8;
const int HAIR_LEFT_COUNT = 56;
const int HAIR_LEFT_ITEM_H = 96; // 96 porque se apilan de a 3 (c/u de altura 32)
const int HAIR_RIGHT_X = 128; 
const int HAIR_RIGHT_ITEM_H = 128; // para la parte derecha de la imagen se apilan de a 4

const int SHIRT_PER_ROW = 16;
const int SHIRT_ITEM_H = 32;

const int HAT_PER_ROW = 12;
const int HAT_ITEM_H = 80; // se apilan de a 4 (c/u de altura 20)

const int FEMALE_DY = 1;

// dibujar en orden Body -> Pants -> Shirt -> Hair -> Hat

const uint16_t COLORS[COLOR_COUNT][3] = {
    {40, 40, 50}, // negro
    {110, 70, 40}, // marrón
    {200, 160, 80}, // dorado
    {190, 70, 40}, // naranja
    {240, 240, 240}, // blanco
    {60, 110, 200}, // azul
    {80, 170, 90}, // verde
    {200, 90, 160} // rosa
};

struct Rect {
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;

    Rect(int x, int y, int w, int h) : 
        x(static_cast<uint16_t>(x)), 
        y(static_cast<uint16_t>(y)), 
        width(static_cast<uint16_t>(w)), 
        height(static_cast<uint16_t>(h)) {}
};

class AppearanceTranslator {
  public:
    explicit AppearanceTranslator(CharacterAppearance& appearance);
    
    Rect body_rect();
    Rect hair_rect();
    Rect shirt_rect();
    Rect pants_rect();
    Rect hat_rect();
  private:
    const CharacterAppearance& appearance;
};

#endif
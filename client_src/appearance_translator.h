#ifndef APPEARANCE_TRANSLATOR_H
#define APPEARANCE_TRANSLATOR_H

#include <stdint.h>

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

const int SHIRT_DX = 4;
const int SHIRT_DY = 13;
const int HAIR_DX = 0;
const int HAIR_DY = -2;
const int HAT_DX = -2;
const int HAT_DY = -4;
const int PANTS_DX = 0;
const int PANTS_DY = -1;

const int FEMALE_DY = 1;

// todos los valores que mas sobresalen
const int CANVAS_X = HAT_DX;
const int CANVAS_Y = HAT_DY;
const int CANVAS_W = HAT_WIDTH;
const int CANVAS_H = BODY_HEIGHT - HAT_DY;

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

#endif
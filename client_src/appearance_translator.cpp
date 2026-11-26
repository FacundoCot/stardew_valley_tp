#include "appearance_translator.h"

AppearanceTranslator::AppearanceTranslator(CharacterAppearance& app) : appearance(app) {}

Translator::Rect AppearanceTranslator::body_rect() {
    int x = appearance.body * BODY_BLOCK_W;
    int y = BODY_ROW * BODY_HEIGHT;
    return Translator::Rect(x, y, BODY_WIDTH, BODY_HEIGHT);
}

Translator::Rect AppearanceTranslator::pants_rect() {
    int x = appearance.pants * PANTS_BLOCK_W;
    int y = PANTS_ROW * PANTS_HEIGHT;
    return Translator::Rect(x, y, PANTS_WIDTH, PANTS_HEIGHT);
}

Translator::Rect AppearanceTranslator::hair_rect() {
    int i = appearance.hair - 1; // hair en cero es calvo (por defecto)

    int x, y;

    if (i < HAIR_LEFT_COUNT) {
        x = (i % HAIR_PER_ROW) * HAIR_WIDTH;
        y = (i / HAIR_PER_ROW) * HAIR_LEFT_ITEM_H;
        return Translator::Rect(x, y, HAIR_WIDTH, HAIR_HEIGHT);
    }

    i -= HAIR_LEFT_COUNT; // indice dentro de la zona derecha

    x = (i % HAIR_PER_ROW) * HAIR_WIDTH;
    y = (i / HAIR_PER_ROW) * HAIR_RIGHT_ITEM_H;

    return Translator::Rect(HAIR_RIGHT_X + x, y, HAIR_WIDTH, HAIR_HEIGHT);
}

Translator::Rect AppearanceTranslator::shirt_rect() {
    int i = appearance.shirt;

    int x = (i % SHIRT_PER_ROW) * SHIRT_WIDTH;
    int y = (i / SHIRT_PER_ROW) * SHIRT_ITEM_H;

    return Translator::Rect(x, y, SHIRT_WIDTH, SHIRT_HEIGHT);
}

Translator::Rect AppearanceTranslator::hat_rect() {
    // retrocedo uno porque hat en cero es sin sombrero (por defecto)
    int i = appearance.hat - 1;

    int x = (i % HAT_PER_ROW) * HAT_WIDTH;
    int y = (i / HAT_PER_ROW) * HAT_ITEM_H;

    return Translator::Rect(x, y, HAT_WIDTH, HAT_HEIGHT);
}


#include "characterpreview.h"

#include <algorithm>
#include <iostream>
#include <QPainter>

CharacterPreview::CharacterPreview(QWidget* parent)
    : QWidget(parent), translator(appearance),
      body_sheet(load_sheet("bodies.png")),
      pants_sheet(load_sheet("pants.png")),
      shirts_sheet(load_sheet("shirts.png")),
      hair_sheet(load_sheet("hairstyles.png")),
      hats_sheet(load_sheet("hats.png")) {
}

// llamo por cada button
void CharacterPreview::prev_body() { prev_part(appearance.body, BODY_COUNT); }
void CharacterPreview::next_body() { next_part(appearance.body, BODY_COUNT); }
void CharacterPreview::prev_hair() { prev_part(appearance.hair, HAIR_COUNT); }
void CharacterPreview::next_hair() { next_part(appearance.hair, HAIR_COUNT); }
void CharacterPreview::prev_shirt() { prev_part(appearance.shirt, SHIRT_COUNT); }
void CharacterPreview::next_shirt() { next_part(appearance.shirt, SHIRT_COUNT); }
void CharacterPreview::prev_pants() { prev_part(appearance.pants, PANTS_COUNT); }
void CharacterPreview::next_pants() { next_part(appearance.pants, PANTS_COUNT); }
void CharacterPreview::prev_hat() { prev_part(appearance.hat, HAT_COUNT); }
void CharacterPreview::next_hat() { next_part(appearance.hat, HAT_COUNT); }
void CharacterPreview::prev_haircolor() { prev_part(appearance.hair_color, COLOR_COUNT); }
void CharacterPreview::next_haircolor() { next_part(appearance.hair_color, COLOR_COUNT); }
void CharacterPreview::prev_pantscolor() { prev_part(appearance.pants_color, COLOR_COUNT); }
void CharacterPreview::next_pantscolor() { next_part(appearance.pants_color, COLOR_COUNT); }

void CharacterPreview::prev_part(uint16_t& part, const int count)  { 
    part = cycle(part, -1, count); 
    update(); 
}

void CharacterPreview::next_part(uint16_t& part, const int count)  { 
    part = cycle(part, +1, count); 
    update(); 
}

CharacterAppearance CharacterPreview::get_appearance() const { return appearance; }

int CharacterPreview::cycle(int item, int change, int amount) {
    int next = (item + change) % amount;
    if (next < 0) 
        next += amount;
    return next;
}

QPixmap CharacterPreview::load_sheet(const QString& file) {
    QString path = "client_src/data/assets/Characters/StardewValley/" + file;
    QPixmap sheet(path);

    if (sheet.isNull()) {
        std::cerr << "No se pudo cargar stylesheet:" << path.toStdString();
    }

    return sheet;
}

QPixmap CharacterPreview::tint(const QPixmap& sprite, int color_index) {
    QImage img = sprite.toImage(); // paso a QImage para poder modificar pixeles puntualmente
    for (int y = 0; y < img.height(); y++) {
        for (int x = 0; x < img.width(); x++) {
            QColor pixel = img.pixelColor(x, y);
            pixel.setRed(pixel.red() * COLORS[color_index][0] / 255);
            pixel.setGreen(pixel.green() * COLORS[color_index][1] / 255);
            pixel.setBlue(pixel.blue() * COLORS[color_index][2] / 255);
            img.setPixelColor(x, y, pixel);
        }
    }
    return QPixmap::fromImage(img);
}

void CharacterPreview::draw_layer(QPainter& painter, const QPixmap& sheet,
                                  const QRect& rect_part, int dx, int dy, int color_index) {
    QPixmap individual_sprite = sheet.copy(rect_part);

    if (color_index >= 0) {
        individual_sprite = tint(individual_sprite, color_index);
    }

    QRect target(origin_x + dx * scale, origin_y + dy * scale,
                 rect_part.width() * scale, rect_part.height() * scale);

    painter.drawPixmap(target, individual_sprite);
}

void CharacterPreview::paintEvent(QPaintEvent*) {
    QPainter painter(this);

    scale = std::max(1, std::min(width() / CANVAS_W, height() / CANVAS_H));
    origin_x = (width() - CANVAS_W * scale) / 2 - CANVAS_X * scale;
    origin_y = (height() - CANVAS_H * scale) / 2 - CANVAS_Y * scale;

    int female_dy = (appearance.body == 1) ? FEMALE_DY : 0;

    draw_layer(painter, body_sheet, to_QRect(translator.body_rect()) , 0, 0, -1); // -1 por no ser "pintable"
    draw_layer(painter, pants_sheet, to_QRect(translator.pants_rect()), PANTS_DX, PANTS_DY + female_dy, appearance.pants_color);
    draw_layer(painter, shirts_sheet, to_QRect(translator.shirt_rect()), SHIRT_DX, SHIRT_DY + female_dy, -1);
    if (appearance.hair > 0) {
        draw_layer(painter, hair_sheet, to_QRect(translator.hair_rect()), HAIR_DX, HAIR_DY + female_dy, appearance.hair_color);
    }
    if (appearance.hat > 0) {
        draw_layer(painter, hats_sheet, to_QRect(translator.hat_rect()), HAT_DX, HAT_DY + female_dy, -1);
    }
}

QRect CharacterPreview::to_QRect(Translator::Rect rect){
    return QRect(rect.x, rect.y, rect.width, rect.height);
}

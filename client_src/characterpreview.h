#ifndef CHARACTERPREVIEW_H
#define CHARACTERPREVIEW_H

#include <QWidget>
#include <QPixmap>
#include "../common_src/character_appearance.h"
#include <QRect>
#include "appearance_translator.h"

const int SHIRT_DX = 4;
const int SHIRT_DY = 13;
const int HAIR_DX = 0;
const int HAIR_DY = -2;
const int HAT_DX = -2;
const int HAT_DY = -4;
const int PANTS_DX = 0;
const int PANTS_DY = -1;

// todos los valores que mas sobresalen
const int CANVAS_X = HAT_DX;
const int CANVAS_Y = HAT_DY;
const int CANVAS_W = HAT_WIDTH;
const int CANVAS_H = BODY_HEIGHT - HAT_DY;

class CharacterPreview : public QWidget {
    public:
        explicit CharacterPreview(QWidget* parent = nullptr);

        CharacterAppearance get_appearance() const;

    public slots:
        void prev_body();    
        void next_body();
        void prev_hair();    
        void next_hair();
        void prev_shirt();   
        void next_shirt();
        void prev_pants();   
        void next_pants();
        void prev_hat();     
        void next_hat();
        void prev_haircolor();  
        void next_haircolor();
        void prev_pantscolor(); 
        void next_pantscolor();

    protected:
        void paintEvent(QPaintEvent *event) override;

    private:
        CharacterAppearance appearance;
        AppearanceTranslator translator;
        QPixmap body_sheet, pants_sheet, shirts_sheet, hair_sheet, hats_sheet;

        int scale = 1;
        int origin_x = 0;
        int origin_y = 0;

        void prev_part(uint16_t& part, const int count);
        void next_part(uint16_t& part, const int count);

        QPixmap load_sheet(const QString& file);

        int cycle(int item, int change, int amount);

        QPixmap tint(const QPixmap& sprite, int color_index);

        void draw_layer(QPainter& painter, const QPixmap& sheet, const QRect& src,
                    int dx, int dy, int color_index);

        QRect to_QRect(Rect rect);
};

#endif
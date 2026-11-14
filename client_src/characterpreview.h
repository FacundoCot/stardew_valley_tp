#ifndef CHARACTERPREVIEW_H
#define CHARACTERPREVIEW_H

#include <QWidget>
#include <QPixmap>
#include "../common_src/character_appearance.h"
#include <QRect>

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
        QPixmap body_sheet, pants_sheet, shirts_sheet, hair_sheet, hats_sheet;
        uint16_t hair_color = 0;
        uint16_t pants_color = 4;

        int scale = 1;
        int origin_x = 0;
        int origin_y = 0;

        void prev_part(uint16_t& part, const int count);
        void next_part(uint16_t& part, const int count);

        QPixmap load_sheet(const QString& file);

        int cycle(int item, int change, int amount);

        QRect body_rect();
        QRect pants_rect();
        QRect hair_rect();
        QRect shirt_rect();
        QRect hat_rect();

        QPixmap tint(const QPixmap& sprite, int color_index);

        void draw_layer(QPainter& painter, const QPixmap& sheet, const QRect& src,
                    int dx, int dy, int color_index);
};

#endif
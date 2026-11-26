#include "menu_in_game.h"
#include "../common_src/messages.h"
#include <SDL2pp/Texture.hh>

MenuInGame::MenuInGame(Visual& visual, Loader& loader) : visual(visual), 
                                                         loader(loader),
                                                         menu_t(loader.get_texture("menu_in_game")),
                                                         btn_enable_multiplayer(loader.get_texture("enable"), 0, 0, 0, 0),
                                                         btn_exit(loader.get_texture("exit"), 0, 0, 0, 0),
                                                         box_code_t(loader.get_texture("empty")),
                                                         font("client_src/data/fonts/dayfont.ttf", 40) {

    float scale = 0.5f;
    int dest_w = static_cast<int>(menu_t.GetWidth()* scale);
    int dest_h = static_cast<int>(menu_t.GetHeight() * scale);

    int dest_x = (visual.get_window().GetWidth() - dest_w) / 2 ;
    int dest_y = (visual.get_window().GetHeight() - dest_h) / 2 ;

    menu = SDL2pp::Rect(dest_x, dest_y, dest_w, dest_h);

    int margin = 20;
    int btn_w = static_cast<int>(loader.get_texture("exit").GetWidth() * 0.65);
    int btn_h = static_cast<int>(loader.get_texture("exit").GetHeight() * scale);
    int btn_x = dest_x + (dest_w - btn_w) / 2;

    btn_enable_multiplayer.set_rect(btn_x, dest_y + margin, btn_w, btn_h);
    btn_exit.set_rect(btn_x, dest_y + btn_h + margin*2, btn_w, btn_h);
}

void MenuInGame::toggle() {
    is_open = !is_open;
}


void MenuInGame::display_menu() {
    if (!is_open) {return;}

    visual.draw_black_overlay();
    visual.get_renderer().Copy(menu_t, SDL2pp::NullOpt, menu);

    if (!is_multiplayer_enabled) {
        btn_enable_multiplayer.draw(visual);
    } else {
        int box_w = static_cast<int>(menu.w * 0.36);
        int box_h = static_cast<int>(menu.h * 0.18);

        SDL2pp::Rect code_box (
            menu.x + (menu.w - box_w) / 2,
            menu.y + static_cast<int>(menu.h* 0.1),
            box_w,
            box_h
        );

        visual.get_renderer().Copy(box_code_t, SDL2pp::NullOpt, code_box);

        if (code_texture.has_value()) {
            int tex_w = code_texture->GetWidth();
            int tex_h = code_texture->GetHeight();

            SDL2pp::Rect dest(
                code_box.x + (code_box.w - tex_w) / 2,
                code_box.y + (code_box.h - tex_h) / 2,
                tex_w,
                tex_h
            );
            visual.get_renderer().Copy(*code_texture, SDL2pp::NullOpt, dest);
        }
    }
    btn_exit.draw(visual);

}

bool MenuInGame::is_menu_open() const {
    return is_open;
}

MenuAction MenuInGame::handle_mouse_event(const SDL_Event& e) {
    if (e.type == SDL_MOUSEMOTION) {
        btn_enable_multiplayer.update_is_hovered(e.motion.x, e.motion.y);
        btn_exit.update_is_hovered(e.motion.x, e.motion.y);

    } else if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
        int mx = static_cast<int>(e.button.x);
        int my = static_cast<int>(e.button.y);

        if (btn_enable_multiplayer.is_clicked(mx, my)) {
            return MenuAction::ENABLE_MULTP;
        }
        if (btn_exit.is_clicked(mx, my)) {
            return MenuAction::EXIT;
        }
    }
    return MenuAction::NONE;
}

void MenuInGame::update(const std::string& game_code) {
    is_multiplayer_enabled = true;
    set_game_code(game_code);
}

void MenuInGame::set_game_code(const std::string& game_code) {
    SDL_Color color = {185,75,75,255};
    std::string code = "Codigo: " + game_code;
    SDL2pp::Surface surface = font.RenderText_Solid(code, color);

    code_texture.emplace(visual.get_renderer(), surface);
}
#include "chat_view.h"

ChatView::ChatView(Visual& visual, Loader& loader) : visual(visual), background(loader.get_texture("menu_in_game")),
                                                     btn_chat(loader.get_texture("btn_chat"), 0, 0, 0, 0) {

    Dimensions btn;
    DimensionsStruct btn_dimensions = btn.get_health_stick(visual);
    

    int btn_w = loader.get_texture("btn_chat").GetWidth()*scale;
    int btn_h = loader.get_texture("btn_chat").GetWidth()*scale;

    int btn_x = visual.get_window().GetWidth() - btn_w - margin;
    int btn_y = btn_dimensions.y + padding;

    btn_chat.set_rect(btn_x, btn_y, btn_w, btn_h);

}


void ChatView::toggle() {
    is_open = !is_open;
    if (is_open) {
        SDL_StartTextInput();
    } else {
        SDL_StopTextInput();
    }
}

void ChatView::display_chat() {
    if (!is_open) {return;}

    visual.draw_black_overlay();
    
    float scale = 0.5f;

    int dest_w = static_cast<int>(background.GetWidth() * scale);
    int dest_h = static_cast<int>(background.GetHeight() * scale);

    int dest_x = (visual.get_window().GetWidth() - dest_w) / 2;
    int dest_y = (visual.get_window().GetHeight() - dest_h) / 2;

    background_chat = SDL2pp::Rect(dest_x, dest_y, dest_w, dest_h);

    visual.get_renderer().Copy(background, SDL2pp::NullOpt, background_chat);

    int text_x = background_chat.x + 30;
    int text_y = background_chat.y + background_chat.h - 50;

    int start_x = background_chat.x + 30;
    int start_y = background_chat.y + 30;
    int line_height = 40;

    for (uint8_t i = 0; i < history.size(); i++) {
        int line_y = start_y + static_cast<int>(i)* line_height;
        visual.draw_text_cached(
            "chat_line_" + std::to_string(i),
            history[i],
            start_x, 
            line_y,
            70, 40, 20
        );
    }

    visual.draw_text_cached("chat_input", "> " + input_text + "-", text_x, text_y, 70, 40, 20);

}

bool ChatView::handle_click(const SDL_Event& e) {
    if (e.type == SDL_MOUSEMOTION) {
        btn_chat.update_is_hovered(e.motion.x, e.motion.y);
    } 
    else if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
        int mx = static_cast<int>(e.button.x);
        int my = static_cast<int>(e.button.y);

        if (btn_chat.is_clicked(mx, my)) {
            return true;
        }
    }
    return false;
}

std::optional<std::string> ChatView::handle_text_event(const SDL_Event& e) {
    if (e.type == SDL_TEXTINPUT) {
        if (input_text.size() < MAX_LENGTH) {
            input_text += e.text.text;
        }
    } else if (e.type == SDL_KEYDOWN) {
        if (e.key.keysym.sym == SDLK_BACKSPACE && !input_text.empty()) {
            input_text.pop_back();
        } else if (e.key.keysym.sym == SDLK_RETURN || e.key.keysym.sym == SDLK_KP_ENTER) {
            if (!input_text.empty()) {
                std::string msg_to_send = std::move(input_text);
                input_text.clear();
                return msg_to_send;
            }
        } else if (e.key.keysym.sym == SDLK_ESCAPE && e.key.repeat) {
            toggle();
        }
    }
    return std::nullopt;
}

void ChatView::add_message(const std::string& sender, const std::string& text) {
    history.push_back(sender + ": " + text);
    if (history.size() > MAX_VISIBLE_MESSAGES) {
        history.pop_front();
    }
}

void ChatView::draw_button() {
    btn_chat.draw(visual);
}

bool ChatView::is_chat_open() const {
    return is_open;
}
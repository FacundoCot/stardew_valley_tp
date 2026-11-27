#ifndef CHAT_VIEW_H 
#define CHAT_VIEW_H

#include "button.h"
#include "visual.h"
#include "loader.h"
#include "dimensions.h"

#include <SDL2pp/Texture.hh>
#include <SDL2pp/Rect.hh>
#include <optional>
#include <string>
#include <deque>


class ChatView {

private:
Visual& visual;
SDL2pp::Texture& background;
SDL2pp::Rect background_chat;
Button btn_chat;
std::string input_text;
std::deque<std::string> history;

static constexpr size_t MAX_VISIBLE_MESSAGES = 10;

bool is_open = false;

const int scale = 3;
const int margin = 20;
const int padding = 10;

static constexpr size_t MAX_LENGTH = 60;
public:

ChatView(Visual& visual, Loader& loader);

void display_chat();
void toggle();
void draw_button();
bool is_chat_open() const;
void add_message(const std::string& sendr, const std::string& text);
bool handle_click(const SDL_Event& e);
std::optional<std::string> handle_text_event(const SDL_Event&e);


};




#endif
#ifndef MENU_IN_GAME_H
#define MENU_IN_GAME_H

#include "visual.h"
#include "loader.h"
#include "button.h"
#include "../../common_src/messages.h"
#include <SDL2/SDL.h>
#include <string>

enum class MenuAction {NONE, ENABLE_MULTP, EXIT};

class MenuInGame {

private:

Visual& visual;
Loader& loader;
bool is_open = false;
bool is_multiplayer_enabled = false;

SDL2pp::Texture& menu_t;
Button btn_enable_multiplayer;
Button btn_exit;
SDL2pp::Rect menu;

SDL2pp::Texture& box_code_t;

SDL2pp::Font font;
std::optional<SDL2pp::Texture> code_texture;


int SRC_X_MENU = 0;
int SRC_Y_MENU = 0;

int SRC_X_EXIT = 0;
int SRC_Y_EXIT = 0;

void set_game_code(const std::string& game_code);

public:
MenuInGame(Visual& visual, Loader& loader);

void display_menu();
void toggle();
void update(const std::string& game_code);
bool is_menu_open() const;
MenuAction handle_mouse_event(const SDL_Event& e);

};

#endif
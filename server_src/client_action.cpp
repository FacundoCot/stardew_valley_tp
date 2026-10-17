#include "client_action.h"

#include <utility>

#include "game_action_handler.h"

CreateGameAction::CreateGameAction(std::string game_name,
                                   std::string player_name):
        game_name(std::move(game_name)),
        player_name(std::move(player_name)) {}

void CreateGameAction::execute(GameActionHandler& handler) const {
    handler.create_game(get_client_id(), game_name, player_name);
}

JoinGameAction::JoinGameAction(std::string game_code,
                               std::string player_name):
        game_code(std::move(game_code)),
        player_name(std::move(player_name)) {}

void JoinGameAction::execute(GameActionHandler& handler) const {
    handler.join_game(get_client_id(), game_code, player_name);
}

MoveAction::MoveAction(Direction direction):
        direction(direction) {}

void MoveAction::execute(GameActionHandler& handler) const {
    handler.move_player(get_client_id(), direction);
}

void DisconnectAction::execute(GameActionHandler& handler) const {
    handler.disconnect_client(get_client_id());
}
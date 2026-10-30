#ifndef CLIENT_ACTION_H
#define CLIENT_ACTION_H

#include <string>

#include "../common_src/messages.h"

class GameActionHandler;

class ClientAction {
private:
  int client_id = -1;

protected:
  // protected para que las clases hijas puedan acceder a este metodo
  // ya que el client_id es privado y no queremos que sea publico para todas las
  // clases
  int get_client_id() const { return client_id; }

public:
  void set_client_id(int id) { client_id = id; }

  // virtual sirve para permitir el polimorfismo, es decir,
  // que podamos llamar a un método de una clase base y que se
  // ejecute la implementación de la clase hija correspondiente.

  // = 0 indica que es un método puro virtual, es decir,
  // que no tiene implementación en la clase base y que debe ser implementado en
  // las clases hijas.

  // ponemos el const para que no se pueda modificar el objeto al que apunta el
  // puntero,
  //  es decir, que no se pueda modificar el objeto desde la clase hija
  virtual void execute(GameActionHandler &handler) const = 0;

  // Destructor virtual: asegura que al destruir un objeto mediante un puntero
  // a la clase padre, se ejecuten también los destructores de las clases hijas.
  virtual ~ClientAction() = default;
};

class CreateGameAction : public ClientAction {
private:
  std::string game_name;
  std::string player_name;

public:
  CreateGameAction(std::string game_name, std::string player_name);

  void execute(GameActionHandler &handler) const override;
};

class JoinGameAction : public ClientAction {
private:
  std::string game_code;
  std::string player_name;

public:
  JoinGameAction(std::string game_code, std::string player_name);

  void execute(GameActionHandler &handler) const override;
};

class MoveAction : public ClientAction {
private:
  Direction direction;

public:
  explicit MoveAction(Direction direction);

  void execute(GameActionHandler &handler) const override;
};

class DisconnectAction : public ClientAction {
public:
  void execute(GameActionHandler &handler) const override;
};

#endif


#include "server_protocol.h"

#include <limits>
#include <stdexcept>
#include <utility>
#include <cmath>

#include <arpa/inet.h>


ServerProtocol::ServerProtocol(Socket& skt): skt(skt) {}

// Recibe `size` bytes desde el socket y los guarda en `data`.
// Si el cliente se desconecta antes de recibir todos los bytes, lanza una
// excepción.
void ServerProtocol::recv_or_throw(void *data, unsigned int size) {
  if (size > 0 && skt.recvall(data, size) == 0) {
    throw std::runtime_error("client disconnected while receiving");
  }
}

// Envía `size` bytes desde `data` al socket.
// Si el cliente se desconecta antes de enviar todos los bytes, lanza una
// excepción.
void ServerProtocol::send_or_throw(const void *data, unsigned int size) {
  if (size > 0 && skt.sendall(data, size) == 0) {
    throw std::runtime_error("client disconnected while sending");
  }
}

std::string ServerProtocol::recv_string() {
  // leo 2 bytes que representan el tamaño de la cadena en red
  // y los guardo en network_size
  uint16_t network_size;
  recv_or_throw(&network_size, sizeof(network_size));

  // los tranformo al endian de la maquina y los guardo en size
  uint16_t size = ntohs(network_size);

  if (size > 1024) {
    throw std::runtime_error("string too long");
  }

  // creo un string de tamaño size y lo lleno con '\0'
  std::string value(size, '\0');
  // recibo size bytes y los guardo en value.data()
  recv_or_throw(value.data(), size);

  return value;
}

void ServerProtocol::send_string(const std::string &value) {
  if (value.size() > 1024) {
    throw std::runtime_error("string too long");
  }

  // tranformo a red de 2 bytes el tamaño de la cadena y lo guardo en
  // network_size
  const uint16_t network_size = htons(value.size());

  // envio los 2 bytes de network_size y luego envio la cadena
  send_or_throw(&network_size, sizeof(network_size));
  send_or_throw(value.data(), value.size());
}

std::shared_ptr<ClientAction> ServerProtocol::recv_action() {
  uint8_t op;
  recv_or_throw(&op, sizeof(op));

    switch (static_cast<ActionType>(op)) {
        // caso move, 1 byte de direction
        case ActionType::MOVE: {
            uint8_t direction;
            recv_or_throw(&direction, sizeof(direction));

            if (direction < static_cast<uint8_t>(Direction::UP) ||
                direction > static_cast<uint8_t>(Direction::RIGHT)) {
                throw std::runtime_error("invalid direction");
            }

            return std::make_shared<MoveAction>(
                    static_cast<Direction>(direction));
            break;
        }
        case ActionType::ATTACK:
            return std::make_shared<AttackAction>();

        case ActionType::DISCONNECT:
            return std::make_shared<DisconnectAction>();
        
        case ActionType::CHAT: {
            std::string text = recv_string();
            //creo una ChatAction, que hereda de client_action
            return std::make_shared<ChatAction>(std::move(text));
        }

        default:
            throw std::runtime_error("invalid game operation");
    }

  }  

static uint32_t coordinate_to_network(float value) {
    int32_t coordinate = static_cast<int32_t>(std::round(value * 100.0f));

    return htonl(static_cast<uint32_t>(coordinate));
}

static void validate_enemies(const std::vector<EnemyState>& enemies) {
    for (const EnemyState& enemy : enemies) {
        if (enemy.health < 0 ||
            enemy.health > std::numeric_limits<uint16_t>::max()) {
            throw std::runtime_error("invalid enemy health value");
        }
        if (enemy.max_health < 0 ||
            enemy.max_health > std::numeric_limits<uint16_t>::max()) {
            throw std::runtime_error("invalid enemy max_health value");
        }
    }
}

void ServerProtocol::send_inventory_state(const InventoryState& inventory) {
    if (inventory.slot_vector.size() > 9) {
        throw std::runtime_error("invalid inventory size");
    }

    // Enviar monedas
    const uint32_t coins_network = htonl(inventory.coins);
    send_or_throw(&coins_network, sizeof(coins_network));

    // Enviar cantidad de slots ocupados
    const uint8_t count = static_cast<uint8_t>(inventory.slot_vector.size());
    send_or_throw(&count, sizeof(count));

    // Enviar cada slot
    for (const InventorySlotState& slot : inventory.slot_vector) {
        //convierto los enums en su valor numerico para enviarlo por el socket
        const uint8_t type = static_cast<uint8_t>(slot.type);
        const uint8_t quality = static_cast<uint8_t>(slot.quality);

        send_or_throw(&slot.index, sizeof(slot.index));
        send_or_throw(&type, sizeof(type));
        send_or_throw(&quality, sizeof(quality));
        send_or_throw(&slot.quantity, sizeof(slot.quantity));
    }
}

void ServerProtocol::send_time_state(const TimeState& time) {
    const uint16_t day_network = htons(time.day);
    send_or_throw(&day_network, sizeof(day_network));

    send_or_throw(&time.hour, sizeof(time.hour));
    send_or_throw(&time.minute, sizeof(time.minute));

    const uint8_t finished = time.finished ? static_cast<uint8_t>(DayFinished::FINISHED) : 
                                             static_cast<uint8_t>(DayFinished::NOT_FINISHED);
    send_or_throw(&finished, sizeof(finished));
}

void ServerProtocol::send_chat_message(const ChatState& chat) {
    const uint8_t kind = static_cast<uint8_t>(chat.kind);

    send_or_throw(&kind, sizeof(kind));

    send_string(chat.sender);
    send_string(chat.text);
}

void ServerProtocol::send_games_list(const std::vector<GameInfoForMenu>& games) {
    uint16_t count = htons(static_cast<uint16_t>(games.size()));
    send_or_throw(&count, sizeof(count));

    for (const GameInfoForMenu& game : games) {
        send_string(game.game_code);
        send_string(game.game_name);
        send_string(game.owner_name);

        uint8_t multiplayer = game.multiplayer ? 1 : 0;
        send_or_throw(&multiplayer, sizeof(multiplayer));
    }
}

void ServerProtocol::send_message(const ServerMessage& message) {
    if (message.type == MessageType::GAME_STATE) {
        validate_enemies(message.enemies);
    }

    const uint8_t type = static_cast<uint8_t>(message.type);
    send_or_throw(&type, sizeof(type));

    switch (message.type) {
        case MessageType::GAME_CREATED:
        case MessageType::GAME_JOINED: {
          send_string(message.game_code);
          break;
        }

        case MessageType::GAME_STATE: {
            /*
             * GAME_STATE:
             * u8  tipo de mensaje = 3
             * u16 cantidad de jugadores (htons)
             * por cada jugador:
             *   string nombre (u16 longitud en htons + bytes)
             *   int32 x (valor * 100, htonl)
             *   int32 y (valor * 100, htonl)
             *   u16 vida (htons)
             *   u16 vida maxima (htons)
             *   u8  inconsciente (0 o 1)
             * u16 cantidad de enemigos (htons)
             * por cada enemigo:
             *   u8  tipo (EnemyKind)
             *   int32 x (valor * 100, htonl)
             *   int32 y (valor * 100, htonl)
             *   u16 vida (htons)
             *   u16 vida maxima (htons)
             */
            if (message.players.size() >
                std::numeric_limits<uint16_t>::max()) {
                throw std::runtime_error("too many players");
            }

            const uint16_t count =
                htons(static_cast<uint16_t>(message.players.size()));
            send_or_throw(&count, sizeof(count));

          for (const PlayerState& player : message.players) {
            send_string(player.player_name);

            const uint32_t x = coordinate_to_network(player.x);
            const uint32_t y = coordinate_to_network(player.y);

            send_or_throw(&x, sizeof(x));
            send_or_throw(&y, sizeof(y));

            if (player.health < 0 || player.max_health < 0 ||
                player.health > std::numeric_limits<uint16_t>::max() ||
                player.max_health > std::numeric_limits<uint16_t>::max()) {
                  throw std::runtime_error("invalid health value");
                }
                
            const uint16_t health = htons(static_cast<uint16_t>(player.health));
            const uint16_t max_health = htons(static_cast<uint16_t>(player.max_health));

            send_or_throw(&health, sizeof(health));
            send_or_throw(&max_health, sizeof(max_health));

            const uint8_t unconscious = player.unconscious ? 1 : 0;
            send_or_throw(&unconscious, sizeof(unconscious));
            }
            if (message.enemies.size() >
                std::numeric_limits<uint16_t>::max()) {
                throw std::runtime_error("too many enemies");
            }

            const uint16_t enemy_count =
                htons(static_cast<uint16_t>(message.enemies.size()));
            send_or_throw(&enemy_count, sizeof(enemy_count));

            for (const EnemyState& enemy : message.enemies) {
                const uint8_t kind = static_cast<uint8_t>(enemy.kind);
                send_or_throw(&kind, sizeof(kind));

                const uint32_t x = coordinate_to_network(enemy.x);
                const uint32_t y = coordinate_to_network(enemy.y);
                send_or_throw(&x, sizeof(x));
                send_or_throw(&y, sizeof(y));

                const uint16_t health = htons(static_cast<uint16_t>(enemy.health));
                const uint16_t max_health =
                    htons(static_cast<uint16_t>(enemy.max_health));
                send_or_throw(&health, sizeof(health));
                send_or_throw(&max_health, sizeof(max_health));
            }
            break;
        }
        case MessageType::INVENTORY_STATE:
          send_inventory_state(message.inventory);
          break;
        case MessageType::TIME_STATE:
          send_time_state(message.time);
          break;
        case MessageType::CHAT_MESSAGE:
          send_chat_message(message.chat);
          break;
        case MessageType::GAMES_LIST:
          send_games_list(message.games);
          break;
        case MessageType::ERROR:
          break;
        default:
          throw std::runtime_error("invalid server message");
  }
}

MenuRequest ServerProtocol::recv_menu_request() {
    uint8_t op;
    recv_or_throw(&op, sizeof(op));

    switch (static_cast<ActionType>(op)) {
        case ActionType::CREATE_GAME: {
          std::string game_name = recv_string();
          std::string player_name = recv_string();
          CharacterAppearance appearance = recv_appearance();

          return MenuRequest{ActionType::CREATE_GAME, "por_defecto",
                             std::move(game_name), "", std::move(player_name),
                             appearance};
        }

        case ActionType::JOIN_GAME: {
          std::string game_code = recv_string();
          std::string player_name = recv_string();
          CharacterAppearance appearance = recv_appearance();

          return MenuRequest{ActionType::JOIN_GAME, "por_defecto", "",
                             std::move(game_code), std::move(player_name),
                             appearance};
        }
        case ActionType::LIST_GAMES:
          return MenuRequest{ActionType::LIST_GAMES, "por_defecto", "", "",
                             "", {}};

        default:
            throw std::runtime_error("invalid menu request");
    }
}

CharacterAppearance ServerProtocol::recv_appearance(){
  uint16_t parts[5] = {};
  for(uint16_t& p : parts){
    recv_or_throw(&p, sizeof(uint16_t));
    p = ntohs(p);
  }
  
  return CharacterAppearance{parts[0], parts[1], parts[2], parts[3], parts[4]};
}

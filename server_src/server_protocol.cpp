
#include "server_protocol.h"

#include <limits>
#include <stdexcept>
#include <utility>
#include <cmath>

#include <arpa/inet.h>

#include "../common_src/world_serialization.h"

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

          return std::make_shared<MoveAction>(static_cast<Direction>(direction));
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

        case ActionType::CONFIRM_NEXT_DAY:
          return std::make_shared<ConfirmNextDayAction>();

        case ActionType::SELECT_SLOT: {
          uint8_t slot_index;
          recv_or_throw(&slot_index, sizeof(slot_index));

          if (slot_index >= 9) {
            throw std::runtime_error("invalid slot index");
          }

          return std::make_shared<SelectSlotAction>(slot_index);
        }
        case ActionType::ENABLE_MULTIPLAYER:
          return std::make_shared<EnableMultiplayerAction>();
        
        case ActionType::INTERACT_MERCHANT: {
          uint8_t merchant_type;
          recv_or_throw(&merchant_type, sizeof(merchant_type));

          if (merchant_type > static_cast<uint8_t>(MerchantType::CLINT)) {
            throw std::runtime_error("invalid merchant");
          }

          return std::make_shared<InteractMerchantAction>(static_cast<MerchantType>(merchant_type));
        }

        case ActionType::TILL_SOIL:
        case ActionType::PLANT_SEED:
        case ActionType::WATER_CROP:
        case ActionType::HARVEST_CROP: {
            uint16_t network_x;
            uint16_t network_y;

            recv_or_throw(&network_x, sizeof(network_x));
            recv_or_throw(&network_y, sizeof(network_y));

            const int x = ntohs(network_x);
            const int y = ntohs(network_y);

            switch (static_cast<ActionType>(op)) {
                case ActionType::TILL_SOIL:
                    return std::make_shared<TillSoilAction>(x, y);

                case ActionType::PLANT_SEED:
                    return std::make_shared<PlantSeedAction>(x, y);

                case ActionType::WATER_CROP:
                    return std::make_shared<WaterCropAction>(x, y);

                case ActionType::HARVEST_CROP:
                    return std::make_shared<HarvestCropAction>(x, y);

                default:
                    throw std::runtime_error("invalid crop operation");
            }
        }
          
        default:
            throw std::runtime_error("invalid game operation");
    }

  }  

static uint32_t coordinate_to_network(float value) {
    int32_t coordinate = static_cast<int32_t>(std::round(value * 100.0f));

    return htonl(static_cast<uint32_t>(coordinate));
}

static void validate_variant(EnemyVariant variant) {
    if (static_cast<uint8_t>(variant) > static_cast<uint8_t>(EnemyVariant::GREEN_FLAME)) {
        throw std::runtime_error("invalid enemy variant");
    }
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
        validate_variant(enemy.variant);
    }
}

void ServerProtocol::send_inventory_state(const InventoryState& inventory) {
    if (inventory.slot_vector.size() > 9) {
        throw std::runtime_error("invalid inventory size");
    }

    // Enviar monedas
    const uint32_t coins_network = htonl(inventory.coins);
    send_or_throw(&coins_network, sizeof(coins_network));

    //enviar slot seleccionado
    send_or_throw(&inventory.selected_slot, sizeof(inventory.selected_slot));

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

void ServerProtocol::send_zone_id(ZoneId zone) {
  const uint8_t kind = static_cast<uint8_t>(zone.kind);
  const uint8_t level = static_cast<uint8_t>(zone.level);

  send_or_throw(&kind, sizeof(kind));
  send_or_throw(&level, sizeof(level));
}

void ServerProtocol::send_chat_message(const ChatState& chat) {
  const uint8_t kind = static_cast<uint8_t>(chat.kind);

  send_or_throw(&kind, sizeof(kind));

  send_string(chat.sender);
  send_string(chat.text);
}

void ServerProtocol::send_player_state(const PlayerState& player) {
  send_string(player.player_name);

  const uint32_t x = coordinate_to_network(player.x);
  const uint32_t y = coordinate_to_network(player.y);

  send_or_throw(&x, sizeof(x));
  send_or_throw(&y, sizeof(y));

  const uint16_t health = htons(static_cast<uint16_t>(player.health));
  const uint16_t max_health = htons(static_cast<uint16_t>(player.max_health));
  const uint16_t energy = htons(static_cast<uint16_t>(player.energy));
  const uint16_t max_energy = htons(static_cast<uint16_t>(player.max_energy));

  send_or_throw(&health, sizeof(health));
  send_or_throw(&max_health, sizeof(max_health));
  send_or_throw(&energy, sizeof(energy));
  send_or_throw(&max_energy, sizeof(max_energy));

  const uint8_t unconscious = player.unconscious ? 1 : 0;
  send_or_throw(&unconscious, sizeof(unconscious));

  send_appearance(player.appearance);

  const uint8_t facing = static_cast<uint8_t>(player.facing);
  send_or_throw(&facing, sizeof(facing));
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

        send_string(game.player_name);
        send_appearance(game.appearance);
    }
}

void ServerProtocol::send_world_info(const World& world) {
        /*
     * WORLD_INFO:
     * u8  tipo de mensaje = 7
     * u32 tamaño del bloque que sigue (htonl)
     * bloque:
     *   u8  version del formato = 8
     *   u8  cantidad de zonas
     *   por cada zona:
     *     u8 kind, u8 level
     *     u16 ancho, u16 alto (htons)
     *     ancho * alto bytes de tiles (0=wall, 1=floor,
     *       2=grass, 3=path, 4=tree)
     *     u8 cantidad de portales
     *     por cada portal: u8 tipo, i32 celda x, i32 celda y,
     *       u8 kind destino, u8 level destino, i32 llegada x * 100,
     *       i32 llegada y * 100
     *     datos propios de granja o pueblo
     */
    const uint8_t type = static_cast<uint8_t>(MessageType::WORLD_INFO);
    send_or_throw(&type, sizeof(uint8_t));

    const std::vector<uint8_t> data = encode_world(world);
    const uint32_t size = htonl(static_cast<uint32_t>(data.size()));
    send_or_throw(&size, sizeof(uint32_t));
    send_or_throw(data.data(), static_cast<unsigned int>(data.size()));
}

void ServerProtocol::send_message(const ServerMessage& message) {
    if (message.type == MessageType::GAME_STATE) {
        validate_enemies(message.enemies);
    }

    const uint8_t type = static_cast<uint8_t>(message.type);
    send_or_throw(&type, sizeof(type));

    switch (message.type) {
        case MessageType::GAME_CREATED:
        case MessageType::GAME_JOINED:
        case MessageType::MULTIPLAYER_ENABLED: {
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
             *   u8 variante, u8 frenesí
             * u16 cantidad de proyectiles (htons)
             * por cada proyectil: u8 variante, int32 x, int32 y
             *   u8  inconsciente (0 o 1)
             * u16 cantidad de enemigos (htons)
             * por cada enemigo:
             *   u8  tipo (EnemyKind)
             *   int32 x (valor * 100, htonl)
             *   int32 y (valor * 100, htonl)
             *   u16 vida (htons)
             *   u16 vida maxima (htons)
             */
            // Zona y personaje del destinatario
            send_zone_id(message.zone);
            send_player_state(message.self_player);

            // Otros jugadores visibles
            const uint16_t count = htons(static_cast<uint16_t>(message.players.size()));

            send_or_throw(&count, sizeof(count));

            for (const PlayerState& player : message.players) {
              send_player_state(player);
            }

            // Jugadores para el mapa general
            const uint16_t map_count = htons(static_cast<uint16_t>(message.map_players.size()));

            send_or_throw(&map_count, sizeof(map_count));

            for (const MapPlayerState& player : message.map_players) {
              send_string(player.player_name);

              const uint32_t x = coordinate_to_network(player.x);
              const uint32_t y = coordinate_to_network(player.y);

              send_or_throw(&x, sizeof(x));
              send_or_throw(&y, sizeof(y));

              send_zone_id(player.zone);
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
                validate_variant(enemy.variant);
                const uint8_t variant = static_cast<uint8_t>(enemy.variant);
                const uint8_t frenzy = enemy.frenzy ? 1 : 0;
                send_or_throw(&variant, sizeof(variant));
                send_or_throw(&frenzy, sizeof(frenzy));
            }

            if (message.projectiles.size() > 512) {
                throw std::runtime_error("too many projectiles");
            }
            const uint16_t projectile_count =
                htons(static_cast<uint16_t>(message.projectiles.size()));
            send_or_throw(&projectile_count, sizeof(projectile_count));
            for (const ProjectileState& projectile : message.projectiles) {
                validate_variant(projectile.variant);
                const uint8_t variant = static_cast<uint8_t>(projectile.variant);
                send_or_throw(&variant, sizeof(variant));
                const uint32_t x = coordinate_to_network(projectile.x);
                const uint32_t y = coordinate_to_network(projectile.y);
                send_or_throw(&x, sizeof(x));
                send_or_throw(&y, sizeof(y));
            }

          send_inventory_state(message.inventory);
          send_time_state(message.time);
          send_farm_plots(message.farm_plots);

          const uint8_t multiplayer_enabled = message.multiplayer_enabled ? 1 : 0;
          send_or_throw(&multiplayer_enabled, sizeof(multiplayer_enabled));

          break;
        }
        /*
        case MessageType::INVENTORY_STATE:
          send_inventory_state(message.inventory);
          break;
        case MessageType::TIME_STATE:
          send_time_state(message.time);
          break;
        */
        case MessageType::CHAT_MESSAGE:
          send_chat_message(message.chat);
          break;
        case MessageType::GAMES_LIST:
          send_games_list(message.games);
          break;
        case MessageType::USERNAME:
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
          CharacterAppearance appearance = recv_appearance();

          return MenuRequest{ActionType::CREATE_GAME, "por_defecto",
                             std::move(game_name), "", "",
                             appearance};
        }

        case ActionType::JOIN_GAME: {
          std::string game_code = recv_string();
          CharacterAppearance appearance = recv_appearance();

          return MenuRequest{ActionType::JOIN_GAME, "por_defecto", "",
                             std::move(game_code), "",
                             appearance};
        }
        case ActionType::LIST_GAMES:
          return MenuRequest{ActionType::LIST_GAMES, "por_defecto", "", "",
                             "", {}};
        
        case ActionType::USERNAME: {
            std::string name = recv_string();
            return MenuRequest{ActionType::USERNAME, "por defecto", "", "", name, CharacterAppearance{}};
          }
        default:
            throw std::runtime_error("invalid menu request");
    }
}


/*std::string ClientProtocol::serialize_username(const ClientMessage &msg_to_server) {
  size_t len_msg = sizeof(uint8_t) + sizeof(uint16_t) + msg_to_server.player_name.length();
  std::string buffer;
  buffer.reserve(len_msg);

  buffer += static_cast<uint8_t>(msg_to_server.type);
  buffer += msg_to_server.player_name.length();
  buffer += msg_to_server.player_name;

  return buffer;
}*/

CharacterAppearance ServerProtocol::recv_appearance(){
  uint16_t parts[7] = {};
  for(uint16_t& p : parts){
    recv_or_throw(&p, sizeof(uint16_t));
    p = ntohs(p);
  }
  
  return CharacterAppearance{parts[0], parts[1], parts[2], parts[3], parts[4], parts[5], parts[6]};
}

void ServerProtocol::send_appearance(const CharacterAppearance& appearance){
    send_appearance_part(appearance.body);
    send_appearance_part(appearance.hair);
    send_appearance_part(appearance.shirt);
    send_appearance_part(appearance.pants);
    send_appearance_part(appearance.hat);
    send_appearance_part(appearance.hair_color);
    send_appearance_part(appearance.pants_color);
}

void ServerProtocol::send_appearance_part(const uint16_t part){
    uint16_t temp;
    temp = htons(part);
    send_or_throw(&temp, sizeof(uint16_t));
}

void ServerProtocol::send_farm_plots(const std::vector<FarmPlotState>& plots) {

    const uint16_t count = htons(static_cast<uint16_t>(plots.size()));

    send_or_throw(&count, sizeof(count));

    for (const FarmPlotState& plot : plots) {
        const uint16_t x = htons(plot.cell_x);
        const uint16_t y = htons(plot.cell_y);

        const uint8_t has_crop = plot.has_crop ? 1 : 0;

        const uint8_t crop_type = static_cast<uint8_t>(plot.crop_type);

        const uint16_t grown = htons(plot.days_grown);
        const uint16_t required = htons(plot.growth_days);

        const uint8_t needs_water = plot.needs_water ? 1 : 0;
        const uint8_t ready = plot.ready_to_harvest ? 1 : 0;

        send_or_throw(&x, sizeof(x));
        send_or_throw(&y, sizeof(y));
        send_or_throw(&has_crop, sizeof(has_crop));
        send_or_throw(&crop_type, sizeof(crop_type));
        send_or_throw(&grown, sizeof(grown));
        send_or_throw(&required, sizeof(required));
        send_or_throw(&needs_water, sizeof(needs_water));
        send_or_throw(&ready, sizeof(ready));
    }
}

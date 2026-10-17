
#include "server_protocol.h"

#include <limits>
#include <stdexcept>
#include <utility>

#include <arpa/inet.h>

ServerProtocol::ServerProtocol(Socket& skt): skt(skt) {}

// Recibe `size` bytes desde el socket y los guarda en `data`.
// Si el cliente se desconecta antes de recibir todos los bytes, lanza una excepción.
void ServerProtocol::recv_or_throw(void* data, unsigned int size) {
    if (size > 0 && skt.recvall(data, size) == 0) { 
        throw std::runtime_error("client disconnected while receiving");
    }
}

// Envía `size` bytes desde `data` al socket.
// Si el cliente se desconecta antes de enviar todos los bytes, lanza una excepción.
void ServerProtocol::send_or_throw(const void* data, unsigned int size) {
    if (size > 0 && skt.sendall(data, size) == 0) {
        throw std::runtime_error("client disconnected while sending");
    }
}

std::string ServerProtocol::recv_string() {
    //leo 2 bytes que representan el tamaño de la cadena en red
    //y los guardo en network_size
    uint16_t network_size;
    recv_or_throw(&network_size, sizeof(network_size));

    //los tranformo al endian de la maquina y los guardo en size
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

void ServerProtocol::send_string(const std::string& value) {
    if (value.size() > 1024) {
        throw std::runtime_error("string too long");
    }

    //tranformo a red de 2 bytes el tamaño de la cadena y lo guardo en network_size
    const uint16_t network_size = htons(value.size());

    //envio los 2 bytes de network_size y luego envio la cadena
    send_or_throw(&network_size, sizeof(network_size));
    send_or_throw(value.data(),value.size());
}

std::shared_ptr<ClientAction> ServerProtocol::recv_action() {
    uint8_t op;
    recv_or_throw(&op, sizeof(op));

    switch (op) {
        // caso create, 2 bytes de game_name, string
        // y despues 2 bytes de player_name, string
        case static_cast<uint8_t>(ActionType::CREATE_GAME): {
            std::string game_name = recv_string();
            std::string player_name = recv_string();

            return std::make_shared<CreateGameAction>(
                    std::move(game_name), std::move(player_name));
            break;
            }

        // caso join, 2 bytes de game_code, string
        // y despues 2 bytes de player_name, string
        case static_cast<uint8_t>(ActionType::JOIN_GAME): {
            std::string game_code = recv_string();
            std::string player_name = recv_string();

            return std::make_shared<JoinGameAction>(
                    std::move(game_code), std::move(player_name));
            break;
        }
        // caso move, 1 byte de direction
        case static_cast<uint8_t>(ActionType::MOVE): {
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

        default:
            throw std::runtime_error("invalid operation");
    }
}

void ServerProtocol::send_message(const ServerMessage& message) {
    const uint8_t type = static_cast<uint8_t>(message.type);
    send_or_throw(&type, sizeof(type));

    switch (message.type) {
        case MessageType::GAME_CREATED:
        case MessageType::GAME_JOINED: {
            send_string(message.game_code);

            const uint32_t id = htonl(message.player_id);
            send_or_throw(&id, sizeof(id));
            break;
        }

        case MessageType::GAME_STATE: {
            if (message.players.size() >
                std::numeric_limits<uint16_t>::max()) {
                throw std::runtime_error("too many players");
            }

            const uint16_t count =
                    htons(static_cast<uint16_t>(message.players.size()));

            send_or_throw(&count, sizeof(count));

            for (const PlayerState& player : message.players) {
                const uint32_t id = htonl(player.id);
                const uint16_t x = htons(player.x);
                const uint16_t y = htons(player.y);

                send_or_throw(&id, sizeof(id));
                send_or_throw(&x, sizeof(x));
                send_or_throw(&y, sizeof(y));
            }
            break;
        }

        case MessageType::ERROR:
            send_string(message.error);
            break;

        default:
            throw std::runtime_error("invalid server message");
    }
}

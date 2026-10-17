#include "client_sender.h"

#include <exception>
#include <iostream>

#include "../common_src/messages.h"
#include "server_protocol.h"

ClientSender::ClientSender(Socket& skt,
                           Queue<ServerMessage>& outgoing):
        skt(skt),
        outgoing(outgoing) {}

void ClientSender::run() {
    ServerProtocol protocol(skt);

    try {
        while (should_keep_running()) {
            ServerMessage message = outgoing.pop();

            if (!should_keep_running()) {
                break;
            }

            protocol.send_message(message);
        }
    } catch (const ClosedQueue&) {
        // exepcion que nos indica que la cola fue cerrada.
    } catch (const std::exception& e) {
        if (should_keep_running()) {
            std::cerr << "Error en ClientSender: "<< e.what() << std::endl;
        }
    }
}
/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ServerLink
*/

#ifndef R_TYPE_SERVER_LINK_TPP
#define R_TYPE_SERVER_LINK_TPP

#include "ServerLink.hpp"
#include "rtype/protocol/Header.hpp"
#include "rtype/protocol/MessageType.hpp"

namespace rtype::client {
template <class Message>
void ServerLink::send(protocol::MessageType type, const Message &message)
{
    ++sendSequence;
    socket.sendTo(server, protocol::makePacket(type, sendSequence, message));
}
}

#endif // R_TYPE_SERVER_LINK_TPP

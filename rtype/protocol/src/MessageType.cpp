/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** MessageType
*/

#include "rtype/protocol/MessageType.hpp"

#include <format>
#include <sstream>
#include <string>

#include "rtype/protocol/ProtocolError.hpp"

namespace rtype::protocol {
static std::string describe(MessageType type, const char *what)
{
    std::ostringstream stream;
    stream << type << ": " << what;
    return stream.str();
}

void checkType(MessageType type, Direction expected)
{
    if (!isKnown(type))
        throw ProtocolError(DecodeError::UNKNOWN_TYPE,
            describe(type, ": unknown type"));

    switch (expected) {
    case Direction::CLIENT_TO_SERVER:
        if (isClientToServer(type))
            return;
        break;
    case Direction::SERVER_TO_CLIENT:
        if (isServerToClient(type))
            return;
        break;
    }
    throw ProtocolError(DecodeError::WRONG_DIRECTION,
        describe(type, ": wrong direction"));
}

std::ostream &operator<<(std::ostream &stream, MessageType type)
{
    switch (type) {
    case MessageType::HELLO:
        return stream << "HELLO";
    case MessageType::INPUT:
        return stream << "INPUT";
    case MessageType::DISCONNECT:
        return stream << "DISCONNECT";
    case MessageType::WELCOME:
        return stream << "WELCOME";
    case MessageType::REJECT:
        return stream << "REJECT";
    case MessageType::SNAPSHOT:
        return stream << "SNAPSHOT";
    case MessageType::EVENT:
        return stream << "EVENT";
    case MessageType::SHUTDOWN:
        return stream << "SHUTDOWN";
    case MessageType::KICK:
        return stream << "KICK";
    }

    return stream << std::format("0x{:02x}", static_cast<unsigned>(type));
}
}

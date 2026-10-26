/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** MessageType
*/

#include "rtype/protocol/MessageType.hpp"

#include <format>
#include <string>

namespace rtype::protocol {
constexpr bool isKnown(MessageType type) noexcept
{
    switch (type) {
    case MessageType::HELLO:
    case MessageType::INPUT:
    case MessageType::DISCONNECT:
    case MessageType::WELCOME:
    case MessageType::REJECT:
    case MessageType::SNAPSHOT:
    case MessageType::EVENT:
    case MessageType::SHUTDOWN:
    case MessageType::KICK:
        return true;
    }

    return false;
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

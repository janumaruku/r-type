/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** MessageType
*/

#ifndef R_TYPE_MESSAGE_TYPE_HPP
#define R_TYPE_MESSAGE_TYPE_HPP

#include <cstdint>
#include <ostream>

namespace rtype::protocol {
enum class MessageType : std::uint8_t {
    HELLO = 0x01,
    INPUT = 0x02,
    DISCONNECT = 0x03,
    WELCOME = 0x81,
    REJECT = 0x82,
    SNAPSHOT = 0x83,
    EVENT = 0x84,
    SHUTDOWN = 0x85,
    KICK = 0x86,
};

[[nodiscard]] constexpr bool isKnown(MessageType type) noexcept;

std::ostream &operator<<(std::ostream &stream, MessageType type);
}

#endif //R_TYPE_MESSAGE_TYPE_HPP

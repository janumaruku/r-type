/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ProtocolError
*/

#ifndef R_TYPE_PROTOCOL_ERROR_HPP
#define R_TYPE_PROTOCOL_ERROR_HPP

#include <cstdint>
#include <iosfwd>

#include "network/Packet.hpp"

namespace rtype::protocol {
enum class DecodeError: std::uint8_t {
    BAD_SIZE,
    BAD_MAGIC,
    UNKNOW_TYPE,
    WRONG_DIRECTION,
    TRUNCATED,
    TRAILING_BYTES,
    OUT_OF_RANGE,
};

std::ostream &operator<<(std::ostream &stream, DecodeError error);

class ProtocolError: public net::PacketError {
public:
    ProtocolError(DecodeError code, const std::string &message);

    [[nodiscard]] DecodeError code() const noexcept;

private:
    DecodeError _code;
};

void checkDatagramSize(std::size_t size);
} // protocol
// rtype

#endif //R_TYPE_PROTOCOL_ERROR_HPP

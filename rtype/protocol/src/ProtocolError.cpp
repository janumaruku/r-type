/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ProtocolError
*/

#include "rtype/protocol/ProtocolError.hpp"

#include "rtype/protocol/Header.hpp"

namespace rtype::protocol {
std::ostream &operator<<(std::ostream &stream, DecodeError error)
{
    switch (error) {
    case DecodeError::BAD_SIZE:
        stream << "BAD SIZE";
    case DecodeError::BAD_MAGIC:
        stream << "BAD MAGIC";
    case DecodeError::UNKNOW_TYPE:
        stream << "UNKNOWN TYPE";
    case DecodeError::WRONG_DIRECTION:
        stream << "WRONG DIRECTION";
    case DecodeError::TRUNCATED:
        stream << "TRUNCATED";
    case DecodeError::TRAILING_BYTES:
        stream << "TRAILING BYTES";
    default:
        stream << "UNKNOWN";
    }

    return stream;
}

ProtocolError::ProtocolError(DecodeError code,
    const std::string &message): PacketError(message), _code(code)
{
}

DecodeError ProtocolError::code() const noexcept
{
    return _code;
}

void checkDatagramSize(std::size_t size)
{
    if (size < Header::SIZE || size > net::Packet::MAX_SIZE)
        throw ProtocolError(DecodeError::BAD_SIZE,
            "maformed packet, size out of range");
}
}

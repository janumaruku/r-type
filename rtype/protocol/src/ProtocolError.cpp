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
        return stream << "BAD_SIZE";
    case DecodeError::BAD_MAGIC:
        return stream << "BAD_MAGIC";
    case DecodeError::UNKNOWN_TYPE:
        return stream << "UNKNOWN_TYPE";
    case DecodeError::WRONG_DIRECTION:
        return stream << "WRONG_DIRECTION";
    case DecodeError::TRUNCATED:
        return stream << "TRUNCATED";
    case DecodeError::TRAILING_BYTES:
        return stream << "TRAILING_BYTES";
    case DecodeError::OUT_OF_RANGE:
        return stream << "OUT_OF_RANGE";
    }

    return stream << "UNKNOWN";
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
            "Datagram: size out of range");
}

DecodeError classify(const net::PacketError &error) noexcept
{
    if (const auto *protocolError = dynamic_cast<const ProtocolError *>(&error))
        return protocolError->code();
    return DecodeError::TRUNCATED;
}
}

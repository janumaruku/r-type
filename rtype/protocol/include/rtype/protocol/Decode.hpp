/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Decode
*/

#ifndef R_TYPE_DECODE_HPP
#define R_TYPE_DECODE_HPP

#include "network/Packet.hpp"

namespace rtype::protocol {
/*
** Reads the payload of a message, after its header was read from the same
** packet. Bytes left over throw ProtocolError(TRAILING_BYTES), so dispatchers
** and tests call decode instead of packet.read and apply that rule in one place.
*/
template <net::Serializable Message>
[[nodiscard]] Message decode(net::Packet &packet);
}

#endif //R_TYPE_DECODE_HPP

#include "rtype/protocol/Decode.tpp"

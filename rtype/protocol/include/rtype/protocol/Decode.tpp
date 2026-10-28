/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Decode
*/

#ifndef R_TYPE_DECODE_TPP
#define R_TYPE_DECODE_TPP

#include <sstream>

#include "rtype/protocol/Decode.hpp"
#include "rtype/protocol/MessageType.hpp"
#include "rtype/protocol/ProtocolError.hpp"

namespace rtype::protocol {
template <net::Serializable Message>
Message decode(net::Packet &packet)
{
    Message message = packet.read<Message>();

    if (!packet.fullyRead()) {
        std::ostringstream stream;
        stream << Message::TYPE << ": bytes left over";
        throw ProtocolError(DecodeError::TRAILING_BYTES, stream.str());
    }
    return message;
}
}

#endif //R_TYPE_DECODE_TPP

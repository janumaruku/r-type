/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ClientDispatch
*/

#include "ClientDispatch.hpp"

#include <chrono>
#include <optional>

#include "engine/core/Logger.hpp"
#include "rtype/protocol/MessageType.hpp"

namespace rtype::client {
namespace {
constexpr std::chrono::seconds LOG_INTERVAL{1};
}

bool ClientLogLimiter::shouldLog(protocol::DecodeError code, TimePoint now)
{
    const auto [last, inserted] = _last.try_emplace(code, now);

    if (inserted)
        return true;
    if (now - last->second < LOG_INTERVAL)
        return false;
    last->second = now;
    return true;
}

std::optional<ReceivedMessage> checkFromServer(const net::Datagram &datagram,
    const ServerLink &link, ClientLogLimiter &logs, TimePoint now)
{
    if (datagram.from != link.server)
        return std::nullopt;
    try {
        protocol::checkDatagramSize(datagram.bytes.size());
        ReceivedMessage message{.header = {},
            .packet = net::Packet(datagram.bytes)};

        message.packet >> message.header;
        if (message.header.version != protocol::Header::VERSION)
            return std::nullopt;
        protocol::checkType(message.header.type,
            protocol::Direction::SERVER_TO_CLIENT);
        return message;
    } catch (const net::PacketError &error) {
        if (logs.shouldLog(protocol::classify(error), now))
            engine::core::log::Logger("client").warn(
                "Dropped a datagram from the server: {}", error.what());
        return std::nullopt;
    }
}
}

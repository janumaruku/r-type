/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Messages
*/

#include "rtype/protocol/Messages.hpp"

#include <format>

#include "rtype/protocol/ProtocolError.hpp"

namespace rtype::protocol {
void serialize(net::Packet &packet, const Hello &message)
{
    packet << message.protocolVersion << message.playerName;
}

void deserialize(net::Packet &packet, Hello &message)
{
    packet >> message.protocolVersion >> message.playerName;
    if (message.playerName.size() > MAX_NAME_LENGTH)
        throw ProtocolError(DecodeError::OUT_OF_RANGE,
            "HELLO: playerName too long");
}

void serialize(net::Packet &packet, const Welcome &message)
{
    packet << message.playerId << message.tickRate << message.snapshotRate <<
        message.maxPlayers << message.serverTick;
}

void deserialize(net::Packet &packet, Welcome &message)
{
    packet >> message.playerId >> message.tickRate >> message.snapshotRate >>
        message.maxPlayers >> message.serverTick;

    if (message.playerId >= MAX_PLAYERS)
        throw ProtocolError(DecodeError::OUT_OF_RANGE,
            "WELCOME: playerId out of range");
    if (message.maxPlayers == 0 || message.maxPlayers > MAX_PLAYERS)
        throw ProtocolError(DecodeError::OUT_OF_RANGE,
            "WELCOME: maxPlayers out of range");
    if (message.tickRate == 0)
        throw ProtocolError(DecodeError::OUT_OF_RANGE,
            "WELCOME: tickRate out of range");
    if (message.snapshotRate == 0)
        throw ProtocolError(DecodeError::OUT_OF_RANGE,
            "WELCOME: snapshotRate out of range");
}

void serialize(net::Packet &packet, const Reject &message)
{
    packet << message.reason;
}

void deserialize(net::Packet &packet, Reject &message)
{
    packet >> message.reason;

    switch (message.reason) {
    case RejectReason::SERVER_FULL:
    case RejectReason::VERSION_MISMATCH:
        return;
    }
    throw ProtocolError(DecodeError::OUT_OF_RANGE,
        "REJECT: reason out of range");
}

void serialize(net::Packet &packet, const Input &message)
{
    packet << message.inputSequence << message.buttons;
}

void deserialize(net::Packet &packet, Input &message)
{
    packet >> message.inputSequence >> message.buttons;

    if ((message.buttons & buttons::RESERVED) != 0)
        throw ProtocolError(DecodeError::OUT_OF_RANGE,
            "INPUT: reserved bits set");
}

void serialize(net::Packet &packet, const Disconnect &message)
{
    packet << message.reason;
}

void deserialize(net::Packet &packet, Disconnect &message)
{
    packet >> message.reason;

    switch (message.reason) {
    case DisconnectReason::QUIT:
        return;
    }
    throw ProtocolError(DecodeError::OUT_OF_RANGE,
        "DISCONNECT: reason out of range");
}

void serialize(net::Packet &, const Shutdown &)
{
}

void deserialize(net::Packet &, Shutdown &)
{
}

void serialize(net::Packet &packet, const Kick &message)
{
    packet << message.reason;
}

void deserialize(net::Packet &packet, Kick &message)
{
    packet >> message.reason;

    switch (message.reason) {
    case KickReason::PROTOCOL_ERROR:
    case KickReason::RATE_LIMITED:
        return;
    }
    throw ProtocolError(DecodeError::OUT_OF_RANGE,
        "KICK: reason out of range");
}

std::string sanitizeName(std::string_view name,
    const std::uint8_t playerId)
{
    if (name.empty())
        return std::format("Player {}", playerId + 1);

    name = name.substr(0, MAX_NAME_LENGTH);
    std::string result;

    for (const char letter: name) {
        const auto byte = static_cast<unsigned char>(letter);
        result.push_back(byte < 0x20 || byte > 0x7E ? '?' : letter);
    }

    return result;
}
}

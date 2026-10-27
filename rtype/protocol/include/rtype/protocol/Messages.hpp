/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Messages
*/

#ifndef R_TYPE_MESSAGES_HPP
#define R_TYPE_MESSAGES_HPP

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

#include "rtype/protocol/Header.hpp"
#include "network/Packet.hpp"

namespace rtype::protocol {
inline constexpr std::size_t MAX_NAME_LENGTH = 16;
inline constexpr std::uint8_t MAX_PLAYERS = 4;

struct Hello {
    std::uint8_t protocolVersion = Header::VERSION;
    std::string playerName{};

    bool operator==(const Hello &) const = default;
};

void serialize(net::Packet &packet, const Hello &message);

void deserialize(net::Packet &packet, Hello &message);

struct Welcome {
    std::uint8_t playerId = 0;
    std::uint8_t tickRate = 60;
    std::uint8_t snapshotRate = 30;
    std::uint8_t maxPlayers = MAX_PLAYERS;
    std::uint32_t serverTick = 0;

    bool operator==(const Welcome &) const = default;
};

void serialize(net::Packet &packet, const Welcome &message);

void deserialize(net::Packet &packet, Welcome &message);

enum class RejectReason : std::uint8_t {
    SERVER_FULL = 1,
    VERSION_MISMATCH = 2,
};

struct Reject {
    RejectReason reason = RejectReason::SERVER_FULL;

    bool operator==(const Reject &) const = default;
};

void serialize(net::Packet &packet, const Reject &message);

void deserialize(net::Packet &packet, Reject &message);

namespace buttons {
inline constexpr std::uint8_t UP = 1U << 0U;
inline constexpr std::uint8_t DOWN = 1U << 1U;
inline constexpr std::uint8_t LEFT = 1U << 2U;
inline constexpr std::uint8_t RIGHT = 1U << 3U;
inline constexpr std::uint8_t FIRE = 1U << 4U;
inline constexpr std::uint8_t RESERVED = 0xE0;
}

struct Input {
    std::uint32_t inputSequence = 0;
    std::uint8_t buttons = 0;

    bool operator==(const Input &) const = default;
};

void serialize(net::Packet &packet, const Input &message);

void deserialize(net::Packet &packet, Input &message);

enum class DisconnectReason : std::uint8_t {
    QUIT = 0,
};

struct Disconnect {
    DisconnectReason reason = DisconnectReason::QUIT;

    bool operator==(const Disconnect &) const = default;
};

void serialize(net::Packet &packet, const Disconnect &message);

void deserialize(net::Packet &packet, Disconnect &message);

struct Shutdown {
    bool operator==(const Shutdown &) const = default;
};

void serialize(net::Packet &packet, const Shutdown &message);

void deserialize(net::Packet &packet, Shutdown &message);

enum class KickReason : std::uint8_t {
    PROTOCOL_ERROR = 1,
    RATE_LIMITED = 2,
};

struct Kick {
    KickReason reason = KickReason::PROTOCOL_ERROR;

    bool operator==(const Kick &) const = default;
};

void serialize(net::Packet &packet, const Kick &message);

void deserialize(net::Packet &packet, Kick &message);

[[nodiscard]] std::string sanitizeName(std::string_view name,
    std::uint8_t playerId);
}

#endif //R_TYPE_MESSAGES_HPP

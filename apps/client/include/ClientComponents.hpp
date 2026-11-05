/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ClientComponents.hpp
*/

#ifndef CLIENT_COMPONENTS_HPP
#define CLIENT_COMPONENTS_HPP

#include <cstdint>

#include "engine/core/Vec2.hpp"
#include "rtype/protocol/Decode.hpp"
#include "rtype/protocol/Snapshot.hpp"
#include "rtype/game/Types.hpp"

namespace rtype::client {

struct NetSample {
    std::uint32_t tick;
    engine::core::Vec2f position;
    engine::core::Vec2f velocity;
};

struct NetSamples {
    std::array<NetSample, 3> ring{};
    std::uint8_t count         = 0;
    std::uint32_t lastSeenTick = 0;
};

struct Replica {
    protocol::EntityKind kind;
    std::uint8_t variant;
    game::NetId netId;
};

struct LocalPlayer {};

struct PlayerLabel {
    std::uint8_t playerId;
};

struct HighContrastOutline {};

struct ShipTilt {
    float lastY = 0.0F;
};

} // namespace rtype::client

#endif /* !CLIENT_COMPONENTS_HPP_ */

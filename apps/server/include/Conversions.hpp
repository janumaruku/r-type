/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Conversions.hpp
*/

#ifndef CONVERSIONS_HPP
#define CONVERSIONS_HPP

#include <type_traits>

#include "rtype/protocol/Event.hpp"
#include "rtype/protocol/Snapshot.hpp"
#include "rtype/game/Events.hpp"
#include "rtype/game/Types.hpp"
#include "rtype/game/PlayerRoster.hpp"
#include "rtype/game/GameWorld.hpp"


namespace rtype::server {

constexpr std::uint8_t MAX_PLAYERS = 4;

template <typename Enum>
[[nodiscard]] constexpr std::underlying_type_t<Enum> toUnderlying(Enum value) noexcept
{
    static_assert(std::is_enum_v<Enum>);
    return static_cast<std::underlying_type_t<Enum>>(value);
}

[[nodiscard]] constexpr protocol::EntityKind toProtocol(game::EntityKind kind) noexcept
{
    return static_cast<protocol::EntityKind>(kind);
}

[[nodiscard]] constexpr protocol::LeaveReason toProtocol(game::LeaveReason reason) noexcept
{
    return static_cast<protocol::LeaveReason>(reason);
}

[[nodiscard]] constexpr protocol::PlayerState toProtocol(game::SlotState state) noexcept
{
    return static_cast<protocol::PlayerState>(state);
}

[[nodiscard]] constexpr game::InputButtons toGame(std::uint8_t buttons) noexcept
{
    return static_cast<game::InputButtons>(buttons);
}

[[nodiscard]] protocol::EntityState toProtocol(const game::ReplicatedEntity &entity) noexcept;
[[nodiscard]] std::array<protocol::PlayerInfo, MAX_PLAYERS> rosterToProtocol(const game::PlayerRoster &roster);

struct OutgoingEvent {
    protocol::Event event;
    std::uint8_t copies;
};

[[nodiscard]] OutgoingEvent toProtocol(const game::GameEvent &event);

}

static_assert(rtype::server::toUnderlying(rtype::protocol::EntityKind::PLAYER_SHIP) ==
              rtype::server::toUnderlying(rtype::game::EntityKind::PLAYER_SHIP));

static_assert(rtype::server::toUnderlying(rtype::protocol::EntityKind::BYDOS_SLAVE) ==
              rtype::server::toUnderlying(rtype::game::EntityKind::BYDOS_SLAVE));

static_assert(rtype::server::toUnderlying(rtype::protocol::EntityKind::PLAYER_MISSILE) ==
              rtype::server::toUnderlying(rtype::game::EntityKind::PLAYER_MISSILE));

static_assert(rtype::server::toUnderlying(rtype::protocol::EntityKind::ENEMY_MISSILE) ==
              rtype::server::toUnderlying(rtype::game::EntityKind::ENEMY_MISSILE));


#endif /* !CONVERSIONS_HPP */
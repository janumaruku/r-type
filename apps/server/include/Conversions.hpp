/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Conversions.hpp
*/

#ifndef CONVERSIONS_HPP
#define CONVERSIONS_HPP

#include <type_traits>

#include "rtype/game/Events.hpp"
#include "rtype/game/GameWorld.hpp"
#include "rtype/game/PlayerRoster.hpp"
#include "rtype/game/Types.hpp"
#include "rtype/protocol/Event.hpp"
#include "rtype/protocol/Messages.hpp"
#include "rtype/protocol/Snapshot.hpp"

namespace rtype::server {

using protocol::MAX_PLAYERS;

template <typename Enum>
[[nodiscard]] constexpr std::underlying_type_t<Enum> toUnderlying(Enum value)
    noexcept
{
    static_assert(std::is_enum_v<Enum>);
    return static_cast<std::underlying_type_t<Enum>>(value);
}

[[nodiscard]] constexpr protocol::EntityKind toProtocol(game::EntityKind kind)
    noexcept
{
    return static_cast<protocol::EntityKind>(kind);
}

[[nodiscard]] constexpr protocol::LeaveReason toProtocol(game::LeaveReason
        reason) noexcept
{
    return static_cast<protocol::LeaveReason>(reason);
}

[[nodiscard]] constexpr protocol::PlayerState toProtocol(game::SlotState state)
    noexcept
{
    return static_cast<protocol::PlayerState>(state);
}

/**
 * @brief The protocol's deserialize has already rejected the reserved bits,
 * and the button bits are equal on both sides (checked below), so it's a
 * plain copy.
 */
[[nodiscard]] constexpr game::InputButtons toGame(std::uint8_t buttons) noexcept
{
    return game::InputButtons{.bits = buttons};
}

[[nodiscard]] protocol::EntityState toProtocol(const game::ReplicatedEntity
        &entity) noexcept;
[[nodiscard]] std::array<protocol::PlayerInfo, MAX_PLAYERS> rosterToProtocol(
    const game::PlayerRoster &roster);

struct OutgoingEvent {
    protocol::Event event;
    std::uint8_t copies;
};

[[nodiscard]] OutgoingEvent toProtocol(const game::GameEvent &event);

} // namespace rtype::server

/*
** The game keeps its own copies of these protocol values, since it can't
** include rtype_protocol. The conversions above are plain casts, so each
** pair of values is checked here: if one side changes, this stops compiling.
*/
namespace rtype::server::checks {
template <typename Game, typename Protocol>
constexpr bool same(Game game, Protocol protocol) noexcept
{
    return toUnderlying(game) == toUnderlying(protocol);
}

static_assert(same(game::EntityKind::PLAYER_SHIP,
    protocol::EntityKind::PLAYER_SHIP));
static_assert(same(game::EntityKind::BYDOS_SLAVE,
    protocol::EntityKind::BYDOS_SLAVE));
static_assert(same(game::EntityKind::PLAYER_MISSILE,
    protocol::EntityKind::PLAYER_MISSILE));
static_assert(same(game::EntityKind::ENEMY_MISSILE,
    protocol::EntityKind::ENEMY_MISSILE));

static_assert(same(game::LeaveReason::QUIT, protocol::LeaveReason::QUIT));
static_assert(same(game::LeaveReason::TIMEOUT, protocol::LeaveReason::TIMEOUT));
static_assert(same(game::LeaveReason::KICKED, protocol::LeaveReason::KICKED));

static_assert(same(game::SlotState::EMPTY, protocol::PlayerState::EMPTY));
static_assert(same(game::SlotState::ALIVE, protocol::PlayerState::ALIVE));
static_assert(same(game::SlotState::RESPAWNING,
    protocol::PlayerState::RESPAWNING));
static_assert(same(game::SlotState::SLOT_OUT,
    protocol::PlayerState::PLAYER_OUT));

static_assert(game::InputButtons::UP == protocol::buttons::UP);
static_assert(game::InputButtons::DOWN == protocol::buttons::DOWN);
static_assert(game::InputButtons::LEFT == protocol::buttons::LEFT);
static_assert(game::InputButtons::RIGHT == protocol::buttons::RIGHT);
static_assert(game::InputButtons::FIRE == protocol::buttons::FIRE);
static_assert((protocol::buttons::RESERVED &
                  (game::InputButtons::UP | game::InputButtons::DOWN |
                      game::InputButtons::LEFT | game::InputButtons::RIGHT |
                      game::InputButtons::FIRE)) == 0);
} // namespace rtype::server::checks

#endif /* !CONVERSIONS_HPP */

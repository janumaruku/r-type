/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Conversions.hpp
*/

#ifndef CONVERSIONS_HPP
#define CONVERSIONS_HPP

#include "rtype/protocol/Event.hpp"
#include "rtype/protocol/Snapshot.hpp"
#include "rtype/game/Events.hpp"
#include "rtype/game/Types.hpp"
#include "rtype/game/PlayerRoster.hpp"


namespace rtype::server {

[[nodiscard]] protocol::EntityKind toProtocol(game::EntityKind kind) noexcept;
[[nodiscard]] protocol::LeaveReason toProtocol(game::LeaveReason reason) noexcept;
[[nodiscard]] protocol::PlayerState toProtocol(game::SlotState state) noexcept;
[[nodiscard]] game::InputButtons toGame(std::uint8_t buttons) noexcept;

[[nodiscard]] protocol::EntityState toProtocol(const game::ReplicatedEntity &entity) noexcept;
[[nodiscard]] std::array<protocol::PlayerInfo, 4> rosterToProtocol(const game::PlayerRoster &roster);

struct OutgoingEvent {
    protocol::Event event;
    std::uint8_t copies;
};

[[nodiscard]] OutgoingEvent toProtocol(const game::GameEvent &event);
}

#endif /* !CONVERSIONS_HPP */
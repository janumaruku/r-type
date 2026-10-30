/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** PlayerRoster
*/

#include "rtype/game/PlayerRoster.hpp"

#include <utility>

namespace {
std::string_view toString(const rtype::game::SlotState state)
{
    switch (state) {
    case rtype::game::SlotState::EMPTY:
        return "EMPTY";
    case rtype::game::SlotState::ALIVE:
        return "ALIVE";
    case rtype::game::SlotState::RESPAWNING:
        return "RESPAWNING";
    case rtype::game::SlotState::OUT:
        return "OUT";
    }
    return "UNKNOWN";
}
}

namespace rtype::game {

std::optional<std::uint8_t> PlayerRoster::reserve()
{
    for (std::size_t i = 0; i < _slots.size(); ++i) {
        if (_slots[i].has_value())
            continue;
        const auto playerId = static_cast<std::uint8_t>(i);
        _slots[i].emplace(PlayerSlot{.playerId = playerId});
        return playerId;
    }
    return std::nullopt;
}

void PlayerRoster::release(const std::uint8_t playerId)
{
    if (playerId >= _slots.size())
        return;
    _slots[playerId].reset();
}

bool PlayerRoster::join(const std::uint8_t playerId, std::string name,
    const std::uint8_t lives)
{
    PlayerSlot *slot = expect(playerId, SlotState::EMPTY, "join");

    if (slot == nullptr)
        return false;
    slot->name = std::move(name);
    slot->lives = lives;
    slot->score = 0;
    slot->respawnTimer = 0.0F;
    slot->state = SlotState::RESPAWNING;
    return true;
}

bool PlayerRoster::joinWaiting(const std::uint8_t playerId, std::string name)
{
    PlayerSlot *slot = expect(playerId, SlotState::EMPTY, "joinWaiting");

    if (slot == nullptr)
        return false;
    slot->name = std::move(name);
    slot->lives = 0;
    slot->score = 0;
    slot->state = SlotState::OUT;
    return true;
}

PlayerSlot *PlayerRoster::reserved(const std::uint8_t playerId,
    const std::string_view transition)
{
    if (playerId >= _slots.size() || !_slots[playerId].has_value()) {
        _logger.error("{}: player {} has no reserved slot", transition,
            static_cast<unsigned>(playerId));
        return nullptr;
    }
    return &*_slots[playerId];
}

PlayerSlot *PlayerRoster::expect(const std::uint8_t playerId,
    const SlotState from, const std::string_view transition)
{
    PlayerSlot *slot = reserved(playerId, transition);

    if (slot != nullptr && slot->state != from) {
        _logger.error("{}: player {} is {}, expected {}", transition,
            static_cast<unsigned>(playerId), toString(slot->state),
            toString(from));
        return nullptr;
    }
    return slot;
}
}

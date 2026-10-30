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

bool PlayerRoster::onShipSpawned(const std::uint8_t playerId, const NetId ship)
{
    PlayerSlot *slot = expect(playerId, SlotState::RESPAWNING,
        "onShipSpawned");

    if (slot == nullptr)
        return false;
    slot->shipNetId = ship;
    slot->state = SlotState::ALIVE;
    return true;
}

bool PlayerRoster::onShipKilled(const std::uint8_t playerId,
    const float respawnDelay)
{
    PlayerSlot *slot = expect(playerId, SlotState::ALIVE, "onShipKilled");

    if (slot == nullptr)
        return false;
    if (slot->lives > 0)
        --slot->lives;
    slot->shipNetId = NO_NET_ID;
    if (slot->lives > 0) {
        slot->state = SlotState::RESPAWNING;
        slot->respawnTimer = respawnDelay;
    } else {
        slot->state = SlotState::OUT;
    }
    return true;
}

void PlayerRoster::onGameRestarted(const std::uint8_t lives)
{
    for (auto &slot : _slots) {
        if (!slot.has_value() || slot->state == SlotState::EMPTY)
            continue;
        slot->lives = lives;
        slot->score = 0;
        slot->respawnTimer = 0.0F;
        slot->shipNetId = NO_NET_ID;
        slot->state = SlotState::RESPAWNING;
    }
}

std::vector<std::uint8_t> PlayerRoster::advanceRespawns(const float dt)
{
    std::vector<std::uint8_t> ready;

    for (auto &slot : _slots) {
        if (!slot.has_value() || slot->state != SlotState::RESPAWNING)
            continue;
        slot->respawnTimer = std::max(0.0F, slot->respawnTimer - dt);
        if (slot->respawnTimer <= 0.0F)
            ready.push_back(slot->playerId);
    }
    return ready;
}

bool PlayerRoster::addScore(const std::uint8_t playerId,
    const std::uint32_t points)
{
    PlayerSlot *slot = reserved(playerId, "addScore");

    if (slot == nullptr)
        return false;
    slot->score += points;
    return true;
}

bool PlayerRoster::acceptInput(const std::uint8_t playerId,
    const std::uint32_t inputSequence)
{
    PlayerSlot *slot = reserved(playerId, "acceptInput");

    if (slot == nullptr || inputSequence <= slot->lastInputSequence)
        return false;
    slot->lastInputSequence = inputSequence;
    return true;
}

const PlayerSlot *PlayerRoster::get(const std::uint8_t playerId) const
{
    if (playerId >= _slots.size() || !_slots[playerId].has_value())
        return nullptr;
    return &*_slots[playerId];
}

std::size_t PlayerRoster::active() const noexcept
{
    std::size_t count = 0;

    for (const auto &slot : _slots)
        if (slot.has_value())
            ++count;
    return count;
}

bool PlayerRoster::allOut() const noexcept
{
    bool anyJoined = false;

    for (const auto &slot : _slots) {
        if (!slot.has_value() || slot->state == SlotState::EMPTY)
            continue;
        if (slot->state != SlotState::OUT)
            return false;
        anyJoined = true;
    }
    return anyJoined;
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

/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Conversions.cpp
*/

#include "Conversions.hpp"

#include <algorithm>

#include "rtype/protocol/Event.hpp"

namespace rtype::server {

namespace {

OutgoingEvent makeOutgoing(const protocol::EventType type,
    const std::uint32_t a = 0, const std::uint32_t b = 0, const float x = 0.0F,
    const float y = 0.0F, const std::uint8_t copies = 3)
{
    return {
        .event  = {.type = type, .a = a, .b = b, .x = x, .y = y},
        .copies = copies,
    };
}

struct EventConverter {
    OutgoingEvent operator()(const game::Fired &value) const
    {
        return makeOutgoing(value.side == game::Side::PLAYER
                ? protocol::EventType::PLAYER_FIRED
                : protocol::EventType::ENEMY_FIRED,
            value.shooter, value.missile, value.position.x(),
            value.position.y(), 1);
    }

    OutgoingEvent operator()(const game::EnemyKilled &value) const
    {
        return makeOutgoing(protocol::EventType::ENEMY_KILLED, value.killerShip,
            value.enemy, value.position.x(), value.position.y(), 1);
    }

    OutgoingEvent operator()(const game::PlayerKilled &value) const
    {
        return makeOutgoing(protocol::EventType::PLAYER_KILLED, value.killer,
            value.ship, value.position.x(), value.position.y());
    }

    OutgoingEvent operator()(const game::PlayerJoined &value) const
    {
        return makeOutgoing(protocol::EventType::PLAYER_JOINED, value.playerId);
    }

    OutgoingEvent operator()(const game::PlayerLeft &value) const
    {
        return makeOutgoing(protocol::EventType::PLAYER_LEFT, value.playerId,
            toUnderlying(toProtocol(value.reason)));
    }

    OutgoingEvent operator()(const game::GameOver &value) const
    {
        return makeOutgoing(protocol::EventType::GAME_OVER, value.teamScore);
    }

    OutgoingEvent operator()(const game::GameRestarted &) const
    {
        return makeOutgoing(protocol::EventType::GAME_RESTARTED);
    }
};

} // namespace

protocol::EntityState toProtocol(const game::ReplicatedEntity &entity) noexcept
{
    return {
        .netId   = entity.netId,
        .kind    = toProtocol(entity.kind),
        .variant = entity.variant,
        .x       = entity.position.x(),
        .y       = entity.position.y(),
        .vx      = entity.velocity.x(),
        .vy      = entity.velocity.y(),
    };
}

std::array<protocol::PlayerInfo, MAX_PLAYERS> rosterToProtocol(const game::
        GameWorld &world)
{
    constexpr int MAX_WIRE_HEALTH = 255;
    const auto maxHealth = static_cast<std::uint8_t>(
        std::clamp<int>(world.rules().playerHealth, 0, MAX_WIRE_HEALTH));
    std::array<protocol::PlayerInfo, MAX_PLAYERS> protocolRoster{};

    for (size_t i = 0; i != MAX_PLAYERS; ++i) {
        const auto playerId = static_cast<std::uint8_t>(i);
        const auto *const p = world.roster().get(playerId);
        if (p == nullptr || p->state == game::SlotState::EMPTY) {
            protocolRoster[i].playerId = playerId;
            continue;
        }
        const auto health = world.shipHealth(playerId);
        const bool alive  = p->state == game::SlotState::ALIVE &&
            health.has_value();

        protocolRoster[i] = {
            .playerId  = p->playerId,
            .state     = toProtocol(p->state),
            .lives     = p->lives,
            .health    = alive ? static_cast<std::uint8_t>(std::clamp<int>(
                                     health->current, 0, maxHealth))
                               : std::uint8_t{0},
            .maxHealth = maxHealth,
            .score     = p->score,
            .shipNetId = p->shipNetId,
            .name      = p->name,
        };
    }
    return protocolRoster;
}

[[nodiscard]] OutgoingEvent toProtocol(const game::GameEvent &event)
{
    return std::visit(EventConverter{}, event);
}

protocol::Welcome welcomeFor(std::uint8_t playerId, const game::GameWorld &world)
{
    const game::Rules &rules = world.rules();

    return protocol::Welcome{.playerId = playerId,
        .maxPlayers                    = rules.maxPlayers,
        .playersToStart                = rules.playersToStart,
        .serverTick                    = world.tick(),
        .playerSpeed                   = rules.playerSpeed,
        .playerHalfWidth               = rules.playerHalfSize.x(),
        .playerHalfHeight              = rules.playerHalfSize.y()};
}

} // namespace rtype::server

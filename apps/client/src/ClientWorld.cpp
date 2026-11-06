/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ClientWorld.cpp
*/

#include "ClientWorld.hpp"

#include <algorithm>
#include <utility>
#include <vector>

#include "ClientComponents.hpp"
#include "engine/ecs/Component.hpp"
#include "engine/ecs/Entity.hpp"
#include "rtype/game/Components.hpp"
#include "rtype/game/GameWorld.hpp"
#include "rtype/protocol/Snapshot.hpp"
#include "SpriteCatalog.hpp"

namespace rtype::client {

ClientWorld::ClientWorld(SpriteCatalog sprites)
{
    _registry.setResource(std::move(sprites));
}

void ClientWorld::bindEntityComponents(std::uint32_t tick,
    const protocol::EntityState &state, SpriteCatalog &catalog,
    SpriteKey spriteKey, const NetSample &sample)
{
    const auto entity = _registry.spawnEntity();
    const auto &def   = catalog.find(spriteKey, state.variant);
    _registry.addComponent(entity, rtype::game::Transform{{state.x, state.y}});
    _registry.addComponent(entity, rtype::game::Velocity{{state.vx, state.vy}});
    _registry.addComponent(entity,
        Replica{.kind = state.kind,
            .variant  = state.variant,
            .netId    = state.netId});
    _registry.addComponent(entity,
        NetSamples{.ring{sample}, .lastSeenTick = tick});
    _registry.addComponent(entity, Animation{.def = def});
    _registry.addComponent(entity,
        SpriteDef{
            .texture   = catalog.get(def).texture,
            .states    = catalog.get(def).states,
            .frameTime = catalog.get(def).frameTime,
            .loop      = catalog.get(def).loop,
        });

    if (state.kind == protocol::EntityKind::BYDOS_SLAVE ||
        state.kind == protocol::EntityKind::ENEMY_MISSILE)
        _registry.addComponent(entity, HighContrastOutline{});
    if (state.kind == protocol::EntityKind::PLAYER_SHIP) {
        _registry.addComponent(entity, PlayerLabel{.playerId = state.variant});
        _registry.addComponent(entity, ShipTilt{.lastY = state.y});
    }

    _byNetId.insert_or_assign(state.netId, entity);
}

void ClientWorld::applyEntity(std::uint32_t tick,
    const protocol::EntityState &state)
{
    _latestServerTick = std::max(_latestServerTick, tick);

    auto &catalog  = _registry.getResource<SpriteCatalog>();
    auto spriteKey = static_cast<SpriteKey>(state.kind);
    const NetSample sample{
        .tick     = tick,
        .position = {state.x, state.y},
        .velocity = {state.vx, state.vy},
    };
    auto known = _byNetId.find(state.netId);

    if (known != _byNetId.end()) {
        const auto entity   = known->second;
        auto &net           = *_registry.getComponent<NetSamples>(entity);
        const auto &replica = *_registry.getComponent<Replica>(entity);

        if (replica.kind == state.kind) {
            net.lastSeenTick = std::max(net.lastSeenTick, tick);

            auto &samples = net.ring;
            for (auto &kept: samples) {
                if (kept.tick == tick) {
                    kept = sample;
                    return;
                }
            }
            auto &oldest = *std::ranges::min_element(samples,
                [](const NetSample &a, const NetSample &b) {
                    return a.tick < b.tick;
                });
            if (tick > oldest.tick)
                oldest = sample;
            return;
        }

        engine::core::log::Logger a;
        a.warn("NetId {} changed kind; replacing it",
            std::to_string(state.netId));
        _registry.killEntity(entity);
        _byNetId.erase(known);
    }

    if (!catalog.contains(spriteKey, state.variant)) {
        engine::core::log::Logger a;
        a.warn("Unknown sprite key {} variant {} for entity {}",
            std::to_string(static_cast<std::uint8_t>(state.kind)),
            std::to_string(state.variant), std::to_string(state.netId));
        return;
    }
    bindEntityComponents(tick, state, catalog, spriteKey, sample);
    tagLocalShip();
}

void ClientWorld::applyRoster(std::span<const protocol::PlayerInfo> roster)
{
    std::ranges::copy(roster, _roster.begin());
    auto hasChanged = [this](std::span<const protocol::PlayerInfo> other) {
        for (std::size_t i = 0; i < _roster.size(); ++i) {
            if (_roster[i] != other[i])
                return true;
        }
        return false;
    };
    auto tmp = std::vector<protocol::PlayerInfo>(roster.begin(), roster.end());
    if (std::ranges::find(tmp, _roster[_playerId]) == tmp.end())
        return;
    const_cast<protocol::PlayerInfo &>(roster[_playerId]).shipNetId =
        _roster[_playerId].shipNetId;
    if (hasChanged(roster))
        _rosterChanged = true;
    tagLocalShip();
}

void ClientWorld::tagLocalShip()
{
    auto &locals = _registry.registerComponent<LocalPlayer>();
    const auto ship = localShip();
    const std::vector tagged(locals.entities().begin(),
        locals.entities().end());

    for (const auto &entity: tagged)
        if (!ship || entity != *ship)
            _registry.removeComponent<LocalPlayer>(entity);
    if (ship && !locals.contains(*ship))
        _registry.addComponent(*ship, LocalPlayer{});
}

void ClientWorld::removeNotSeenSince(std::uint32_t tick)
{
    for (auto it = _byNetId.begin(); it != _byNetId.end();) {
        const auto &net = *_registry.getComponent<NetSamples>(it->second);
        if (net.lastSeenTick < tick) {
            _registry.killEntity(it->second);
            it = _byNetId.erase(it);
        } else {
            ++it;
        }
    }
}

void ClientWorld::setLastProcessedInput(std::uint32_t sequence) noexcept
{
    _lastProcessedInput = std::max(_lastProcessedInput, sequence);
}

void ClientWorld::setLocalPlayer(std::uint8_t playerId) noexcept
{
    _playerId = playerId;
}

std::optional<engine::ecs::Entity> ClientWorld::find(game::NetId netId) const
{
    return _byNetId.contains(netId)
        ? std::optional<engine::ecs::Entity>{_byNetId.at(netId)}
        : std::nullopt;
}

std::optional<engine::ecs::Entity> ClientWorld::localShip() const
{
    return find(_roster[_playerId].shipNetId);
}

const std::array<protocol::PlayerInfo, 4> &ClientWorld::roster() const noexcept
{
    return _roster;
}

std::uint32_t ClientWorld::latestServerTick() const noexcept
{
    return _latestServerTick;
}

std::uint32_t ClientWorld::lastProcessedInput() const noexcept
{
    return _lastProcessedInput;
}

bool ClientWorld::rosterChangedSinceLastCall() const noexcept
{
    return _rosterChanged;
}

engine::ecs::Registry &ClientWorld::registry() noexcept
{
    return _registry;
}

} // namespace rtype::client

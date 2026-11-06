/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ClientWorld.hpp
*/

#ifndef CLIENT_WORLD_HPP
#define CLIENT_WORLD_HPP

#include <cstdint>
#include <optional>
#include <span>
#include <unordered_map>

#include "ClientComponents.hpp"
#include "engine/ecs/Entity.hpp"
#include "rtype/game/GameWorld.hpp"
#include "rtype/game/Types.hpp"
#include "rtype/protocol/Snapshot.hpp"
#include "SpriteCatalog.hpp"

namespace rtype::client {

class ClientWorld {
public:
    explicit ClientWorld(SpriteCatalog sprites);

    void applyEntity(std::uint32_t tick, const protocol::EntityState &state);
    void applyRoster(std::span<const protocol::PlayerInfo> roster);
    void removeNotSeenSince(std::uint32_t tick);
    void setLocalPlayer(std::uint8_t playerId) noexcept;
    void setLastProcessedInput(std::uint32_t sequence) noexcept;

    void bindEntityComponents(std::uint32_t tick,
        const protocol::EntityState &state, SpriteCatalog &catalog,
        SpriteKey spriteKey, const NetSample &sample);

    [[nodiscard]] std::optional<engine::ecs::Entity> find(game::NetId netId)
        const;
    [[nodiscard]] std::optional<engine::ecs::Entity> localShip() const;
    [[nodiscard]] game::NetId localShipNetId() const noexcept;
    [[nodiscard]] const std::array<protocol::PlayerInfo, 4> &roster()
        const noexcept;
    [[nodiscard]] std::uint32_t latestServerTick() const noexcept;
    [[nodiscard]] std::uint32_t lastProcessedInput() const noexcept;
    [[nodiscard]] bool rosterChangedSinceLastCall() const noexcept;
    [[nodiscard]] engine::ecs::Registry &registry() noexcept;

private:
    engine::ecs::Registry _registry;
    std::unordered_map<game::NetId, engine::ecs::Entity> _byNetId;
    std::array<protocol::PlayerInfo, 4> _roster{};
    std::uint8_t _playerId            = 0;
    std::uint32_t _latestServerTick   = 0;
    std::uint32_t _lastProcessedInput = 0;
    bool _rosterChanged               = false;
};
} // namespace rtype::client
#endif /* !CLIENT_WORLD_HPP_ */

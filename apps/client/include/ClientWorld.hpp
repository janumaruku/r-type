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
#include "engine/ecs/Entity.hpp"
#include "rtype/game/GameWorld.hpp"
#include "rtype/protocol/Snapshot.hpp"
#include "rtype/game/Types.hpp"


namespace rtype::client {

class SpriteCatalog;

class ClientWorld {
public:
    explicit ClientWorld(SpriteCatalog sprites);

    void applyEntity(std::uint32_t tick, const protocol::EntityState &state);
    void applyRoster(std::span<const protocol::PlayerInfo> roster);
    void removeNotSeenSince(std::uint32_t tick);
    void setLocalPlayer(std::uint8_t playerId) noexcept;
    void setLastProcessedInput(std::uint32_t sequence) noexcept;

    [[nodiscard]] std::optional<engine::ecs::Entity> find(game::NetId netId) const;
    [[nodiscard]] std::optional<engine::ecs::Entity> localShip() const;
    [[nodiscard]] const std::array<protocol::PlayerInfo, 4> &roster() const noexcept;
    [[nodiscard]] std::uint32_t latestServerTick() const noexcept;
    [[nodiscard]] std::uint32_t lastProcessedInput() const noexcept;
    [[nodiscard]] bool rosterChangedSinceLastCall() noexcept;
    [[nodiscard]] engine::ecs::Registry &registry() noexcept;

private:
    engine::ecs::Registry _registry;
    std::unordered_map<game::NetId, engine::ecs::Entity> _byNetId;
    std::array<protocol::PlayerInfo, 4> _roster{};
    std::uint8_t _playerId = 0;
    std::uint32_t _latestServerTick = 0;
    std::uint32_t _lastProcessedInput = 0;
    bool _rosterChanged = false;
};
}
#endif /* !CLIENT_WORLD_HPP_ */
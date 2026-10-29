/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** PlayerRoster
*/

#ifndef R_TYPE_PLAYER_ROSTER_HPP
#define R_TYPE_PLAYER_ROSTER_HPP

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "rtype/game/Types.hpp"

namespace rtype::game {
struct PlayerSlot {
    std::uint8_t playerId = 0;
    std::string name;
    std::uint8_t lives = 0;
    std::uint32_t score = 0;
    SlotState state = SlotState::EMPTY;
    float respawnTimer = 0.0F;
    NetId shipNetId = NO_NET_ID;
    std::uint32_t lastInputSequence = 0;
};

class PlayerRoster {
public:
    [[nodiscard]] std::optional<std::uint8_t> reserve();

    void release(std::uint8_t playerId);

    [[nodiscard]] bool join(std::uint8_t playerId, std::string name,
        std::uint8_t lives);

    [[nodiscard]] bool joinWaiting(std::uint8_t playerId, std::string name);

    [[nodiscard]] bool onShipSpawned(std::uint8_t playerId, NetId ship);

    [[nodiscard]] bool onShipKilled(std::uint8_t playerId, float respawnDelay);

    void onGameRestarted(std::uint8_t lives);

    [[nodiscard]] std::vector<std::uint8_t> advanceRespawns(float dt);

    [[nodiscard]] bool addScore(std::uint8_t playerId, std::uint32_t points);

    [[nodiscard]] bool acceptInput(std::uint8_t playerId,
        std::uint32_t inputSequence);

    [[nodiscard]] const PlayerSlot *get(std::uint8_t playerId) const;

    [[nodiscard]] std::size_t active() const noexcept;

    [[nodiscard]] bool allOut() const noexcept;

    [[nodiscard]] bool anyAlive() const noexcept;

    [[nodiscard]] std::uint32_t teamScore() const noexcept;

private:
    std::array<std::optional<PlayerSlot>, 4> _slots;
};
}

#endif //R_TYPE_PLAYER_ROSTER_HPP

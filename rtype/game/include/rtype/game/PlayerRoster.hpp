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
#include <string_view>

#include "engine/core/Logger.hpp"
#include "rtype/game/Types.hpp"

namespace rtype::game {
struct PlayerSlot {
    std::uint8_t playerId = 0;
    std::string name{};
    std::uint8_t lives = 0;
    std::uint32_t score = 0;
    SlotState state = SlotState::EMPTY;
    float respawnTimer = 0.0F;
    NetId shipNetId = NO_NET_ID;
    std::uint32_t lastInputSequence = 0;
};

class PlayerRoster {
public:
    /**
     * @brief Takes the lowest free slot and leaves it EMPTY, waiting for
     * @ref join or @ref joinWaiting.
     * @return The slot's player id (0-3), or `std::nullopt` when the 4 slots
     * are taken.
     */
    [[nodiscard]] std::optional<std::uint8_t> reserve();

    /**
     * @brief Frees the slot, whatever its state. A free slot or an id >= 4
     * does nothing and logs nothing.
     */
    void release(std::uint8_t playerId);

    /**
     * @brief EMPTY -> RESPAWNING, with `name`, `lives`, a score of 0 and a
     * respawn timer of 0, so the ship spawns on the next tick.
     * @return `false`, with the slot unchanged and an error logged, if the
     * slot isn't reserved or isn't EMPTY.
     */
    [[nodiscard]] bool join(std::uint8_t playerId, std::string name,
        std::uint8_t lives);

    /**
     * @brief EMPTY -> OUT, with `name` and 0 lives: a player joining during
     * a game over waits for @ref onGameRestarted.
     * @return `false`, with the slot unchanged and an error logged, if the
     * slot isn't reserved or isn't EMPTY.
     */
    [[nodiscard]] bool joinWaiting(std::uint8_t playerId, std::string name);

    /**
     * @brief RESPAWNING -> ALIVE, storing the new ship's NetId.
     * @return `false`, with the slot unchanged and an error logged, if the
     * slot isn't reserved or isn't RESPAWNING.
     */
    [[nodiscard]] bool onShipSpawned(std::uint8_t playerId, NetId ship);

    /**
     * @brief ALIVE -> RESPAWNING with `respawnDelay` if a life is left after
     * losing one, OUT otherwise. The ship's NetId goes back to 0.
     * @return `false`, with the slot unchanged and an error logged, if the
     * slot isn't reserved or isn't ALIVE, so a ship hit twice in one tick
     * loses one life only.
     */
    [[nodiscard]] bool onShipKilled(std::uint8_t playerId, float respawnDelay);

    /**
     * @brief Every joined slot (any state but EMPTY) -> RESPAWNING, with
     * `lives`, a score of 0, a respawn timer of 0 and no ship. EMPTY slots
     * keep waiting for their @ref join.
     */
    void onGameRestarted(std::uint8_t lives);

    /**
     * @brief Counts the RESPAWNING timers down by `dt`, stopping at 0.
     * @return The ids whose timer is 0. They stay RESPAWNING until
     * @ref onShipSpawned, so a failed spawn is retried on the next tick.
     */
    [[nodiscard]] std::vector<std::uint8_t> advanceRespawns(float dt);

    /**
     * @brief Adds `points` to the slot's score, in any reserved state: a
     * missile can kill after its shooter died.
     * @return `false`, with an error logged, if the slot isn't reserved.
     */
    [[nodiscard]] bool addScore(std::uint8_t playerId, std::uint32_t points);

    /**
     * @brief Stores `inputSequence` if it's newer than the last one applied
     * (RFC § 6.4), in any reserved state.
     * @return `true` if the input must be applied. A late or duplicate
     * sequence returns `false` without logging; a slot that isn't reserved
     * returns `false` with an error logged.
     */
    [[nodiscard]] bool acceptInput(std::uint8_t playerId,
        std::uint32_t inputSequence);

    /**
     * @brief Read-only access to a slot: changes go through the transitions.
     * @return The slot, or `nullptr` if it's free or the id is >= 4. Valid
     * until the slot is released.
     */
    [[nodiscard]] const PlayerSlot *get(std::uint8_t playerId) const;

    /**
     * @brief The number of reserved slots, EMPTY ones included.
     */
    [[nodiscard]] std::size_t active() const noexcept;

    /**
     * @brief `true` when at least one slot has joined (any state but EMPTY)
     * and every joined slot is OUT: the game is over.
     */
    [[nodiscard]] bool allOut() const noexcept;

    /**
     * @brief `true` when at least one slot is ALIVE, meaning a ship is on
     * screen.
     */
    [[nodiscard]] bool anyAlive() const noexcept;

    /**
     * @brief The sum of the reserved slots' scores.
     */
    [[nodiscard]] std::uint32_t teamScore() const noexcept;

private:
    /**
     * @brief The slot to change, or `nullptr` with an error naming
     * `transition` if it's free or the id is >= 4.
     */
    PlayerSlot *reserved(std::uint8_t playerId, std::string_view transition);

    /**
     * @brief Like @ref reserved, but also `nullptr` with an error naming
     * `transition` and the actual state if the slot isn't in `from`.
     */
    PlayerSlot *expect(std::uint8_t playerId, SlotState from,
        std::string_view transition);

    std::array<std::optional<PlayerSlot>, 4> _slots;
    engine::core::log::Logger _logger{"game"};
};
}

#endif //R_TYPE_PLAYER_ROSTER_HPP

/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** GamePhase
*/

#ifndef R_TYPE_GAME_PHASE_HPP
#define R_TYPE_GAME_PHASE_HPP

#include "rtype/game/Types.hpp"

namespace rtype::game {
class GamePhase {
public:
    [[nodiscard]] Phase phase() const noexcept;

    [[nodiscard]] bool isOver() const noexcept;

    [[nodiscard]] bool enterGameOver(float resetDelay);

    /**
     * @brief WAITING -> PLAYING. Returns false and changes nothing in any
     * other phase.
     */
    [[nodiscard]] bool start();

    /**
     * @brief PLAYING -> WAITING, when every player has left. Returns false
     * and changes nothing in any other phase.
     */
    [[nodiscard]] bool wait();

    /**
     * @brief Counts a GAME_OVER down by `dt`. When it reaches 0, goes to
     * WAITING and returns true, once per game over.
     */
    [[nodiscard]] bool update(float dt);

private:
    Phase _phase      = Phase::WAITING;
    float _untilReset = 0.0F;
};
} // namespace rtype::game

#endif // R_TYPE_GAME_PHASE_HPP

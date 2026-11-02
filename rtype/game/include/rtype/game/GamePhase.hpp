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

    [[nodiscard]] bool update(float dt);

private:
    Phase _phase = Phase::PLAYING;
    float _untilReset = 0.0F;
};
}

#endif //R_TYPE_GAME_PHASE_HPP

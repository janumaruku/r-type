/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** GamePhase
*/

#include "rtype/game/GamePhase.hpp"

namespace rtype::game {

Phase GamePhase::phase() const noexcept
{
    return _phase;
}

bool GamePhase::isOver() const noexcept
{
    return _phase == Phase::GAME_OVER;
}

bool GamePhase::enterGameOver(const float resetDelay)
{
    if (isOver())
        return false;

    _phase = Phase::GAME_OVER;
    _untilReset = resetDelay;
    return true;
}

bool GamePhase::update(const float dt)
{
    if (!isOver())
        return false;

    _untilReset -= dt;
    if (_untilReset > 0.0F)
        return false;

    _phase = Phase::PLAYING;
    _untilReset = 0.0F;
    return true;
}
}

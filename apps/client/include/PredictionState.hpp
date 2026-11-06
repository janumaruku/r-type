/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** PredictionState
*/

#ifndef R_TYPE_PREDICTION_STATE_HPP
#define R_TYPE_PREDICTION_STATE_HPP

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <optional>
#include <utility>

#include "engine/core/Vec2.hpp"
#include "engine/runtime/Clock.hpp"
#include "rtype/game/Rules.hpp"

namespace rtype::client {
struct PendingInput {
    std::uint32_t sequence = 0;
    std::uint8_t buttons = 0;
    engine::runtime::TimePoint sentAt;
};

class PredictionState {
public:
    static constexpr std::size_t MAX_PENDING = 120;
    static constexpr float SNAP_DISTANCE = 64.0F;
    static constexpr float CORRECTION_RATE = 0.2F;

    void record(std::uint32_t sequence, std::uint8_t buttons,
        engine::runtime::TimePoint sentAt);

    [[nodiscard]] engine::core::Vec2f step(engine::core::Vec2f drawn,
        std::uint8_t buttons,
        const game::Rules &rules) const;

    [[nodiscard]] engine::core::Vec2f reconcile(
        engine::core::Vec2f authoritative,
        std::uint32_t lastProcessed,
        const game::Rules &rules);

    [[nodiscard]] engine::core::Vec2f smooth(engine::core::Vec2f drawn,
        engine::core::Vec2f corrected) const;

    [[nodiscard]] std::optional<engine::runtime::TimePoint> sentAt(
        std::uint32_t sequence) const;

    void notePing(std::uint32_t lastProcessed, engine::runtime::TimePoint now);

    [[nodiscard]] std::optional<std::chrono::milliseconds> averagePing(
        engine::runtime::TimePoint now) const;

    [[nodiscard]] std::size_t pending() const noexcept;

private:
    std::deque<PendingInput> _pending;
    std::deque<std::pair<engine::runtime::TimePoint, std::chrono::milliseconds>>
    _pings;
    std::uint32_t _lastPinged = 0;
};
}

#endif //R_TYPE_PREDICTION_STATE_HPP

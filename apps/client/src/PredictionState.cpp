/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** PredictionState.cpp
*/

#include "PredictionState.hpp"

namespace rtype::client {

    void PredictionState::record(std::uint32_t sequence, std::uint8_t buttons, TimePoint sentAt)
    {
        _pending.push_back({sequence, buttons, sentAt});
        if (_pending.size() > MAX_PENDING)
            _pending.pop_front();
    }

    engine::core::Vec2f PredictionState::step(engine::core::Vec2f drawn, std::uint8_t buttons, const game::Rules &rules) const
    {
        engine::core::Vec2f nextFrame = drawn;

        // todo
        // manque l'implémentation des boutons pour faire la suite
        return nextFrame;
    }

    engine::core::Vec2f PredictionState::reconcile(engine::core::Vec2f authoritative, std::uint32_t lastProcessed, const game::Rules &rules)
    {
        engine::core::Vec2f simulated = authoritative;

        while (!_pending.empty() && _pending.front().sequence <= lastProcessed)
            _pending.pop_front();
        for (const auto &input : _pending)
            simulated = step(simulated, input.buttons, rules);
        return simulated;
    }

    engine::core::Vec2f PredictionState::smooth(engine::core::Vec2f drawn, engine::core::Vec2f corrected) const
    {
        float diff_X = corrected.x - drawn.x;
        float diff_Y = corrected.y - drawn.y;
        float distance = std::sqrt(diffX * diff_X + diff_Y * diff_Y);

        if (distance > SNAP_DISTANCE)
            return corrected;
        return {drawn.x + diff_X * CORRECTION_RATE, drawn.y + diff_Y * CORRECTION_RATE};
    }

    std::optional<TimePoint> PredictionState::sentAt(std::uint32_t sequence) const
    {
        for (const auto &p : _pending) {
            if (p.sequence == sequence)
                return p.sentAt;
        }
        return std::nullopt;
    }

    void PredictionState::notePing(std::uint32_t lastProcessed, TimePoint now)
    {
        if (lastProcessed <= _lastPinged)
            return;

        auto time = sentAt(lastProcessed);
        if (time.has_value()) {
            auto rtt = std::chrono::duration_cast<std::chrono::milliseconds>(now - *time);
            _pings.push_back({now, rtt});
            _lastPinged = lastProcessed;
            while (!_pings.empty() && (now - _pings.front().first) > std::chrono::seconds(2))
                _pings.pop_front();
        }
    }

    std::optional<std::chrono::milliseconds> PredictionState::averagePing(TimePoint now) const
    {
        int ms = 0;
        int count = 0;

        if (_pings.empty())
            return std::nullopt;
        for (const auto &p : _pings) {
            if (now - p.first <= std::chrono::seconds(1)) {
                ms += p.count();
                count += 1;
            }
        }
        if (count == 0)
            return std::nullopt;
        return std::chrono::milliseconds(ms / count);
    }

    std::size_t PredictionState::pending() const noexcept
    {
        return _pending.size();
    }

    void predictionSystem(engine::ecs::Registry &registry)
    {
        // todo
    }

}

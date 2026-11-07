/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** PredictionState.cpp
*/

#include "PredictionState.hpp"

// namespace rtype::client {

//     void PredictionState::record(std::uint32_t sequence, std::uint8_t buttons, TimePoint sentAt)
//     {
//         _pending.push_back({sequence, buttons, sentAt});
//         if (_pending.size() > MAX_PENDING)
//             _pending.pop_front();
//     }

//     engine::core::Vec2f PredictionState::step(engine::core::Vec2f drawn, std::uint8_t buttons, const game::Rules &rules) const
//     {
//         engine::core::Vec2f nextFrame = drawn;

//         // todo
//         // manque l'implémentation des boutons pour faire la suite
//         return nextFrame;
//     }

//     engine::core::Vec2f PredictionState::reconcile(engine::core::Vec2f authoritative, std::uint32_t lastProcessed, const game::Rules &rules)
//     {
//         engine::core::Vec2f simulated = authoritative;

//         while (!_pending.empty() && _pending.front().sequence <= lastProcessed)
//             _pending.pop_front();
//         for (const auto &input : _pending)
//             simulated = step(simulated, input.buttons, rules);
//         return simulated;
//     }

//     engine::core::Vec2f PredictionState::smooth(engine::core::Vec2f drawn, engine::core::Vec2f corrected) const
//     {
//         float diff_X = corrected.x - drawn.x;
//         float diff_Y = corrected.y - drawn.y;
//         float distance = std::sqrt(diffX * diff_X + diff_Y * diff_Y);

//         if (distance > SNAP_DISTANCE)
//             return corrected;
//         return {drawn.x + diff_X * CORRECTION_RATE, drawn.y + diff_Y * CORRECTION_RATE};
//     }

//     std::optional<TimePoint> PredictionState::sentAt(std::uint32_t sequence) const
//     {
//         for (const auto &p : _pending) {
//             if (p.sequence == sequence)
//                 return p.sentAt;
//         }
//         return std::nullopt;
//     }

//     void PredictionState::notePing(std::uint32_t lastProcessed, TimePoint now)
//     {
//         if (lastProcessed <= _lastPinged)
//             return;

//         auto time = sentAt(lastProcessed);
//         if (time.has_value()) {
//             auto rtt = std::chrono::duration_cast<std::chrono::milliseconds>(now - *time);
//             _pings.push_back({now, rtt});
//             _lastPinged = lastProcessed;
//             while (!_pings.empty() && (now - _pings.front().first) > std::chrono::seconds(2))
//                 _pings.pop_front();
//         }
//     }

//     std::optional<std::chrono::milliseconds> PredictionState::averagePing(TimePoint now) const
//     {
//         int ms = 0;
//         int count = 0;

//         if (_pings.empty())
//             return std::nullopt;
//         for (const auto &p : _pings) {
//             if (now - p.first <= std::chrono::seconds(1)) {
//                 ms += p.count();
//                 count += 1;
//             }
//         }
//         if (count == 0)
//             return std::nullopt;
//         return std::chrono::milliseconds(ms / count);
//     }

//     std::size_t PredictionState::pending() const noexcept
//     {
//         return _pending.size();
//     }

//     void predictionSystem(engine::ecs::Registry &registry)
//     {
//         // todo
//     }

#include <algorithm>

#include "rtype/game/Motion.hpp"
#include "rtype/game/Types.hpp"

namespace rtype::client {
namespace {
constexpr float STEP_SECONDS = 1.0F / 60.0F;
constexpr auto PING_WINDOW = std::chrono::seconds(1);
}

void PredictionState::record(std::uint32_t sequence, std::uint8_t buttons,
    engine::runtime::TimePoint sentAt)
{
    _pending.push_back(PendingInput{.sequence = sequence, .buttons = buttons,
        .sentAt = sentAt});
    while (_pending.size() > MAX_PENDING)
        _pending.pop_front();
}

engine::core::Vec2f PredictionState::step(engine::core::Vec2f drawn,
    std::uint8_t buttons, const game::Rules &rules) const
{
    return game::movePlayer(drawn, game::InputButtons{.bits = buttons},
        STEP_SECONDS, rules);
}

engine::core::Vec2f PredictionState::reconcile(
    engine::core::Vec2f authoritative, std::uint32_t lastProcessed,
    const game::Rules &rules)
{
    while (!_pending.empty() && _pending.front().sequence <= lastProcessed)
        _pending.pop_front();
    for (const auto &input: _pending)
        authoritative = step(authoritative, input.buttons, rules);
    return authoritative;
}

engine::core::Vec2f PredictionState::smooth(engine::core::Vec2f drawn,
    engine::core::Vec2f corrected)
{
    const engine::core::Vec2f gap = corrected - drawn;

    if (gap.length() > SNAP_DISTANCE)
        return corrected;
    return drawn + gap * CORRECTION_RATE;
}

std::optional<engine::runtime::TimePoint> PredictionState::sentAt(
    std::uint32_t sequence) const
{
    const auto it = std::ranges::find(_pending, sequence,
        &PendingInput::sequence);

    if (it == _pending.end())
        return std::nullopt;
    return it->sentAt;
}

void PredictionState::notePing(std::uint32_t lastProcessed,
    engine::runtime::TimePoint now)
{
    if (lastProcessed <= _lastPinged)
        return;
    const auto sent = sentAt(lastProcessed);

    if (!sent)
        return;
    _lastPinged = lastProcessed;
    _pings.emplace_back(now,
        std::chrono::duration_cast<std::chrono::milliseconds>(now - *sent));
    while (!_pings.empty() && now - _pings.front().first > PING_WINDOW)
        _pings.pop_front();
}

std::optional<std::chrono::milliseconds> PredictionState::averagePing(
    engine::runtime::TimePoint now) const
{
    std::chrono::milliseconds total{0};
    std::int64_t count = 0;

    for (const auto &[at, ping]: _pings) {
        if (now - at > PING_WINDOW)
            continue;
        total += ping;
        ++count;
    }
    if (count == 0)
        return std::nullopt;
    return total / count;
}

std::size_t PredictionState::pending() const noexcept
{
    return _pending.size();
}
}

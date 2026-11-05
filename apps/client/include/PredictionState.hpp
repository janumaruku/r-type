/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** PredictionState.hpp
*/

#ifndef R_TYPE_CLIENTCONFIG_HPP
#define R_TYPE_CLIENTCONFIG_HPP

#include <cstdint>

namespace rtype::client {
    struct PendingInput {
        std::uint32_t sequence;
        std::uint8_t buttons;
        TimePoint sentAt;
    };

    class PredictionState {
        public:
            static constexpr std::size_t MAX_PENDING = 120;
            static constexpr float SNAP_DISTANCE = 64.0F;
            static constexpr float CORRECTION_RATE = 0.2F;

            void record(std::uint32_t sequence, std::uint8_t buttons, TimePoint sentAt);
            [[nodiscard]] Vec2f step(Vec2f drawn, std::uint8_t buttons, const game::Rules &rules) const;
            [[nodiscard]] Vec2f reconcile(Vec2f authoritative, std::uint32_t lastProcessed,
                const game::Rules &rules);
            [[nodiscard]] Vec2f smooth(Vec2f drawn, Vec2f corrected) const;
            [[nodiscard]] std::optional<TimePoint> sentAt(std::uint32_t sequence) const;
            void notePing(std::uint32_t lastProcessed, TimePoint now);
            [[nodiscard]] std::optional<std::chrono::milliseconds> averagePing(TimePoint now) const;
            [[nodiscard]] std::size_t pending() const noexcept;

        private:
            std::deque<PendingInput> _pending;
            std::deque<std::pair<TimePoint, std::chrono::milliseconds>> _pings;
            std::uint32_t _lastPinged = 0;
    };

        struct Correction {
            std::optional<Vec2f> target;
        };

        void predictionSystem(engine::ecs::Registry &registry);



    class ConfigError: public std::runtime_error {
        public:
            using std::runtime_error::runtime_error;
    };

    class HelpRequested: public std::exception {

    };

}

#endif //R_TYPE_CLIENTCONFIG_HPP

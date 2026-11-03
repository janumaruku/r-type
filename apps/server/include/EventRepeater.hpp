/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** EventRepeater.hpp
*/
#ifndef EVENT_REPEATER_HPP
#define EVENT_REPEATER_HPP

#include <vector>

#include "engine/core/Logger.hpp"
#include "rtype/game/GameWorld.hpp"
#include "rtype/protocol/Event.hpp"

namespace rtype::server {

class EventRepeater {
public:
    static constexpr std::size_t MAX_PENDING = 256;

    void add(protocol::Event event, std::uint8_t copies);
    [[nodiscard]] std::vector<protocol::Event> takeDue();

private:
    struct Pending {
        protocol::Event event;
        std::uint8_t copiesLeft;
    };
    std::vector<Pending> _pending;
    std::uint32_t _nextEventId = 1;
    bool _overflowLogged = false;
    engine::core::log::Logger _logger{"EventRepeater"};
};
} // namespace rtype::server

#endif // EVENT_REPEATER_HPP

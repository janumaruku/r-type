/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** EventRepeater.hpp
*/
#ifndef EVENT_REPEATER_HPP
#define EVENT_REPEATER_HPP

#include <vector>

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
};
} // namespace rtype::server

#endif // EVENT_REPEATER_HPP

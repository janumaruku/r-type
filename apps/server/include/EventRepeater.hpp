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
#include "rtype/protocol/Event.hpp"

namespace rtype::server {

/**
 * @brief Numbers the game's events and sends each one as many times as
 * `server.md`'s event table says, one copy per tick, with the same eventId.
 */
class EventRepeater {
public:
    static constexpr std::size_t MAX_PENDING = 256;

    /**
     * @brief Gives `event` the next eventId and keeps it for `copies` ticks.
     * Above @ref MAX_PENDING pending events, the oldest 1-copy event is
     * dropped first, then the oldest of the others, so a burst of kills
     * can't grow memory without limit. An event with 0 copies is ignored
     * and gets no id.
     */
    void add(protocol::Event event, std::uint8_t copies);

    /**
     * @brief One copy of every pending event, oldest first. Each one then
     * has a copy less, and is forgotten after its last copy. Logs one
     * warning with the number of events dropped since the previous call.
     */
    [[nodiscard]] std::vector<protocol::Event> takeDue();

    [[nodiscard]] std::size_t pending() const noexcept;

private:
    struct Pending {
        protocol::Event event;
        std::uint8_t copies;
        std::uint8_t copiesLeft;
    };

    void dropOne();

    std::vector<Pending> _pending;
    std::uint32_t _nextEventId = 1;
    std::size_t _dropped = 0;
    engine::core::log::Logger _logger{"EventRepeater"};
};
} // namespace rtype::server

#endif // EVENT_REPEATER_HPP

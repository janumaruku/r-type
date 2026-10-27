/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Event
*/

#ifndef R_TYPE_EVENT_HPP
#define R_TYPE_EVENT_HPP

#include <cstdint>

#include "network/Packet.hpp"

namespace rtype::protocol {
enum class EventType : std::uint8_t {
    PLAYER_FIRED = 1,
    ENEMY_FIRED = 2,
    ENEMY_KILLED = 3,
    PLAYER_KILLED = 4,
    PLAYER_JOINED = 5,
    PLAYER_LEFT = 6,
    GAME_OVER = 7,
    GAME_RESTARTED = 8,
};

enum class LeaveReason : std::uint8_t {
    QUIT = 0,
    TIMEOUT = 1,
    KICKED = 2,
};

struct Event {
    std::uint32_t eventId = 0;
    EventType type = EventType::PLAYER_FIRED;
    std::uint32_t a = 0;
    std::uint32_t b = 0;
    float x = 0.0F;
    float y = 0.0F;

    bool operator==(const Event &) const = default;
};

void serialize(net::Packet &packet, const Event &message);

void deserialize(net::Packet &packet, Event &message);
}

#endif //R_TYPE_EVENT_HPP

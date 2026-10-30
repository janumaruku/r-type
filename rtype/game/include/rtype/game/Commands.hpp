/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Commands
*/

#ifndef R_TYPE_COMMANDS_HPP
#define R_TYPE_COMMANDS_HPP

#include <cstdint>
#include <string>
#include <variant>

#include "rtype/game/Types.hpp"

namespace rtype::game {
struct JoinPlayer {
    std::uint8_t playerId;
    std::string name;

    bool operator==(const JoinPlayer &) const = default;
};

struct LeavePlayer {
    std::uint8_t playerId;
    LeaveReason reason;

    bool operator==(const LeavePlayer &) const = default;
};

struct SetInput {
    std::uint8_t playerId;
    std::uint32_t inputSequence;
    InputButtons buttons;

    bool operator==(const SetInput &) const = default;
};

using PlayerCommand = std::variant<JoinPlayer, LeavePlayer, SetInput>;
}

#endif //R_TYPE_COMMANDS_HPP

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
};

struct LeavePlayer {
    std::uint8_t playerId;
    LeaveReason reason;
};

struct SetInput {
    std::uint8_t playerId;
    std::uint32_t inputSequence;
    InputButtons buttons;
};

using PlayerCommand = std::variant<JoinPlayer, LeavePlayer, SetInput>;
}

#endif //R_TYPE_COMMANDS_HPP

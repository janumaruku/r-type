/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ClientStages.hpp
*/

#ifndef R_TYPE_CLIENTSTAGES_HPP
#define R_TYPE_CLIENTSTAGES_HPP

#include <cstdint>

namespace rtype::client
{

    enum class ClientStage : std::uint8_t {
        UPDATE, RENDER
    };

}
#endif //R_TYPE_CLIENTSTAGES_HPP

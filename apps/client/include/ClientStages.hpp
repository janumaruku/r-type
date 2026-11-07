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
enum class ClientStage : std::uint8_t { UPDATE, RENDER };
struct RenderClock {
    std::uint32_t latestServerTick = 0;
    float sinceLatestTick = 0.0F;
    [[nodiscard]] float renderTick() const noexcept;
};
inline constexpr std::uint32_t INTERPOLATION_DELAY_TICKS = 6;
inline constexpr float MAX_EXTRAPOLATION_TICKS = 6.0F;
}
#endif //R_TYPE_CLIENTSTAGES_HPP

/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Letterbox.hpp
*/

#ifndef LETTERBOX_HPP
#define LETTERBOX_HPP

#include "engine/core/Rect2.hpp"
#include "engine/core/Vec2.hpp"

namespace engine::render {

/*
** The rectangle, in window pixels, where the logical playfield is drawn:
** scaled by the smaller of the two ratios so it fits whole, and centred with
** equal bars. A window with a zero side (minimised) gives an empty rectangle.
*/
[[nodiscard]] core::FloatRect letterbox(const core::Vec2u &window,
    const core::Vec2f &logical);

}

#endif /* !LETTERBOX_HPP */

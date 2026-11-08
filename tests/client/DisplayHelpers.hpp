/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** DisplayHelpers
*/

#ifndef R_TYPE_DISPLAY_HELPERS_HPP
#define R_TYPE_DISPLAY_HELPERS_HPP

#include <cstdlib>

namespace client_tests {

/**
 * @brief Whether a window can be opened here.
 *
 * Raylib's InitWindow doesn't stop when it finds no display: it goes on to
 * create OpenGL objects without a context and crashes. So a test checks this
 * first and skips. On Linux, a display is an X11 or Wayland server named in
 * the environment, which CI containers don't have. Other systems always have
 * a desktop.
 */
inline bool hasDisplay()
{
#if defined(__linux__)
    return std::getenv("DISPLAY") != nullptr ||
        std::getenv("WAYLAND_DISPLAY") != nullptr;
#else
    return true;
#endif
}

} // namespace client_tests

#endif // R_TYPE_DISPLAY_HELPERS_HPP

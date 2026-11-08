/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** InputState.hpp
*/

#pragma once

#include <bitset>

#include "InputEvent.hpp"
#include "MouseButton.hpp"

namespace engine::input {
class InputState {
public:
    void beginFrame() noexcept;
    void apply(const InputEvent &event) noexcept;
    void releaseAll() noexcept;

    bool isDown(Key key) const noexcept;
    bool wasPressed(Key key) const noexcept;
    bool wasReleased(Key key) const noexcept;
    bool isDown(MouseButton button) const noexcept;
    bool wasPressed(MouseButton button) const noexcept;
    bool wasReleased(MouseButton button) const noexcept;

private:
    std::bitset<KEY_COUNT> _down;
    std::bitset<KEY_COUNT> _pressed;
    std::bitset<KEY_COUNT> _released;
    std::bitset<MOUSE_BUTTON_COUNT> _mouseDown;
    std::bitset<MOUSE_BUTTON_COUNT> _mousePressed;
    std::bitset<MOUSE_BUTTON_COUNT> _mouseReleased;
};
} // namespace engine::input
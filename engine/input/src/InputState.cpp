/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** InputState.cpp
*/

#include "engine/input/InputState.hpp"

namespace engine::input {

void InputState::beginFrame() noexcept
{
    _pressed.reset();
    _released.reset();
    _mousePressed.reset();
    _mouseReleased.reset();
}

void InputState::apply(const InputEvent &event) noexcept
{
    if (event.type == InputEvent::Type::KEY_PRESSED ||
        event.type == InputEvent::Type::KEY_RELEASED) {
        const auto idx = static_cast<std::size_t>(event.key);

        if (event.key == Key::UNKNOWN || idx >= KEY_COUNT)
            return;
        if (event.type == InputEvent::Type::KEY_PRESSED) {
            if (!_down.test(idx)) {
                _down.set(idx);
                _pressed.set(idx);
            }
        } else if (_down.test(idx)) {
            _down.reset(idx);
            _released.set(idx);
        }
        return;
    }
    if (event.type == InputEvent::Type::MOUSE_BUTTON_PRESSED ||
        event.type == InputEvent::Type::MOUSE_BUTTON_RELEASED) {
        const auto idx = static_cast<std::size_t>(event.mouseButton);

        if (idx >= MOUSE_BUTTON_COUNT)
            return;
        if (event.type == InputEvent::Type::MOUSE_BUTTON_PRESSED) {
            if (!_mouseDown.test(idx)) {
                _mouseDown.set(idx);
                _mousePressed.set(idx);
            }
        } else if (_mouseDown.test(idx)) {
            _mouseDown.reset(idx);
            _mouseReleased.set(idx);
        }
    }
}

void InputState::releaseAll() noexcept
{
    for (std::size_t i = 0; i < KEY_COUNT; i++) {
        if (_down.test(i))
            _released.set(i);
    }
    _down.reset();
    for (std::size_t i = 0; i < MOUSE_BUTTON_COUNT; ++i) {
        if (_mouseDown.test(i))
            _mouseReleased.set(i);
    }
    _mouseDown.reset();
}

bool InputState::isDown(Key key) const noexcept
{
    const auto idx = static_cast<std::size_t>(key);

    return idx < KEY_COUNT && _down.test(idx);
}

bool InputState::wasPressed(Key key) const noexcept
{
    const auto idx = static_cast<std::size_t>(key);

    return idx < KEY_COUNT && _pressed.test(idx);
}

bool InputState::wasReleased(Key key) const noexcept
{
    const auto idx = static_cast<std::size_t>(key);

    return idx < KEY_COUNT && _released.test(idx);
}

bool InputState::isDown(const MouseButton button) const noexcept
{
    const auto idx = static_cast<std::size_t>(button);

    return idx < MOUSE_BUTTON_COUNT && _mouseDown.test(idx);
}

bool InputState::wasPressed(const MouseButton button) const noexcept
{
    const auto idx = static_cast<std::size_t>(button);

    return idx < MOUSE_BUTTON_COUNT && _mousePressed.test(idx);
}

bool InputState::wasReleased(const MouseButton button) const noexcept
{
    const auto idx = static_cast<std::size_t>(button);

    return idx < MOUSE_BUTTON_COUNT && _mouseReleased.test(idx);
}

} // namespace engine::input

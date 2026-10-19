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
}

void InputState::apply(const InputEvent &event) noexcept
{
    const auto idx = static_cast<std::size_t>(event.key);

    if (event.key == Key::UNKNOWN)
        return;
    if (idx >= KEY_COUNT)
        return;
    if (event.type == InputEvent::Type::KEY_PRESSED) {
        if (!_down.test(idx)) {
            _down.set(idx);
            _pressed.set(idx);
        }
    }
    if (event.type == InputEvent::Type::KEY_RELEASED) {
        if (_down.test(idx)) {
            _down.reset(idx);
            _released.set(idx);
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

}

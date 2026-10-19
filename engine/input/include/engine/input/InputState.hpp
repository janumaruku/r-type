/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** InputState.hpp
*/

#pragma once

#include "InputEvent.hpp"
#include <bitset>

namespace engine::input {
    class InputState {
        public:
            void beginFrame() noexcept;
            void apply(const InputEvent& event) noexcept;
            void releaseAll() noexcept;

            bool isDown(Key key) const noexcept;
            bool wasPressed(Key key) const noexcept;
            bool wasReleased(Key key) const noexcept;

        private:
            std::bitset<KEY_COUNT> _down;
            std::bitset<KEY_COUNT> _pressed;
            std::bitset<KEY_COUNT> _released;
    };

}

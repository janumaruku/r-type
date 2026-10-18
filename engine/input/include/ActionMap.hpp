/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ActionMap.hpp
*/

#pragma once

#include "Key.hpp"
#include "InputState.hpp"

namespace engine::input
{

    template <typename Action>
    class ActionMap {
        public:
            // bind(Action action, Key key) { _bindings.insert({key, action}); }
            void bind(Action action, Key key) {
                _bindings[key].insert(action);
            }
            void unbind(Action action, Key key) {
                auto i = _bindings.find(key);
                if (i != _bindings.end()) {
                    bindings.erase(i);
                }
            }
            std::vector<Key> keysFor(Action action) const {
                
            }
            bool isActive(Action action, InputState const& state) const {

            }
            bool wasTriggered(Action action, InputState const& state) const {

            }
            bool load(path path) {

            }
            bool save(path path) const {

            }

        private:
            std::unordered_multimap<Key, Action> _bindings;


    };


}

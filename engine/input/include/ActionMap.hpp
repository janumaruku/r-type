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
            void bind(Action action, Key key)
            {
                _bindings[action].insert(key);
            }

            void unbind(Action action, Key key)
            {
                auto i = _bindings.find(action);
                if (i != _bindings.end()) {
                    bindings.erase(i);
                }
            }

            std::vector<Key> keysFor(Action action) const
            {
                std::vector<Key> key;
                auto i = _bindings.find(action);

                if (i != _bindings.end())
                    key.assign(i->second.begin(), i.second.end());
                return key;
            }

            bool isActive(Action action, InputState const& state) const
            {
                
            }

            bool wasTriggered(Action action, InputState const& state) const
            {

            }

            bool load(path path)
            {

            }

            bool save(path path) const
            {

            }

        private:
            std::unordered_multimap<Key, Action> _bindings;


    };


}

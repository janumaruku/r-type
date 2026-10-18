/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ActionMap.hpp
*/

#pragma once

#include "Key.hpp"
#include "InputState.hpp"
#include "json.hpp"
#include <filesystem>

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
                auto i = _bindings.find(action);

                if (i == _bindings.end())
                    return false;
                for (Key key : i->second) {
                    if (state.isDown(key))
                        return true;
                }
                return false;
            }

            bool wasTriggered(Action action, InputState const& state) const
            {
                auto i = _bindings.find(action);

                if (i == _bindings.end())
                    return false;
                for (Key key : i->second) {
                    if (state.wasPressed(key))
                        return true;
                }
                return false;
            }

            bool load(std::filesystem::path path)
            {
                std::ifstream file(path);



                return false;
            }

            bool save(std::filesystem::path path) const
            {
                



                return false;
            }

        private:
            std::unordered_multimap<Key, Action> _bindings;


    };


}

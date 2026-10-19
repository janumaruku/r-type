/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Key.cpp
*/

#include "engine/input/Key.hpp"
#include <optional>
#include <string_view>
#include <array>
#include <cctype>

namespace engine::input
{

    constexpr std::array<std::pair<Key, std::string_view>, KEY_COUNT> KEY_TABLE =
    {
        {
            {Key::UNKNOWN, "UNKNOWN"},
            {Key::KEY_A, "KEY_A"},
            {Key::KEY_B, "KEY_B"},
            {Key::KEY_C, "KEY_C"},
            {Key::KEY_D, "KEY_D"},
            {Key::KEY_E, "KEY_E"},
            {Key::KEY_F, "KEY_F"},
            {Key::KEY_G, "KEY_G"},
            {Key::KEY_H, "KEY_H"},
            {Key::KEY_I, "KEY_I"},
            {Key::KEY_J, "KEY_J"},
            {Key::KEY_K, "KEY_K"},
            {Key::KEY_L, "KEY_L"},
            {Key::KEY_M, "KEY_M"},
            {Key::KEY_N, "KEY_N"},
            {Key::KEY_O, "KEY_O"},
            {Key::KEY_P, "KEY_P"},
            {Key::KEY_Q, "KEY_Q"},
            {Key::KEY_R, "KEY_R"},
            {Key::KEY_S, "KEY_S"},
            {Key::KEY_T, "KEY_T"},
            {Key::KEY_U, "KEY_U"},
            {Key::KEY_V, "KEY_V"},
            {Key::KEY_W, "KEY_W"},
            {Key::KEY_X, "KEY_X"},
            {Key::KEY_Y, "KEY_Y"},
            {Key::KEY_Z, "KEY_Z"},
            {Key::NUM0, "NUM0"},
            {Key::NUM1, "NUM1"},
            {Key::NUM2, "NUM2"},
            {Key::NUM3, "NUM3"},
            {Key::NUM4, "NUM4"},
            {Key::NUM5, "NUM5"},
            {Key::NUM6, "NUM6"},
            {Key::NUM7, "NUM7"},
            {Key::NUM8, "NUM8"},
            {Key::NUM9, "NUM9"},
            {Key::UP, "UP"},
            {Key::DOWN, "DOWN"},
            {Key::LEFT, "LEFT"},
            {Key::RIGHT, "RIGHT"},
            {Key::SPACE, "SPACE"},
            {Key::ENTER, "ENTER"},
            {Key::ESCAPE, "ESCAPE"},
            {Key::LSHIFT, "LSHIFT"},
            {Key::LCTRL, "LCTRL"}
        }
    };

    std::string_view toString(Key key) noexcept
    {
        const auto keyId = static_cast<std::size_t>(key);

        if (keyId < KEY_COUNT)
            return KEY_TABLE[keyId].second;
        return "UNKNOWN";
    }

    std::optional<Key> keyFromString(std::string_view name) noexcept
    {
        char buff[KEY_MAX_SIZE];

        if (name.empty())
            return std::nullopt;
        if (name.size() >= sizeof(buff))
            return std::nullopt;
        for (std::size_t i = 0; i < name.size(); ++i)
            buff[i] = static_cast<char>(std::toupper(static_cast<unsigned char>(name[i])));
        std::string_view upperName(buff, name.size());
        if (upperName == "UNKNOWN" || upperName == "COUNT")
            return std::nullopt;
        for (const auto& [key, keyName] : KEY_TABLE) {
            if (keyName == upperName)
                return key;
        }
        return std::nullopt;
    }

}

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
            {Key::KEY_A, "A"},
            {Key::KEY_B, "B"},
            {Key::KEY_C, "C"},
            {Key::KEY_D, "D"},
            {Key::KEY_E, "E"},
            {Key::KEY_F, "F"},
            {Key::KEY_G, "G"},
            {Key::KEY_H, "H"},
            {Key::KEY_I, "I"},
            {Key::KEY_J, "J"},
            {Key::KEY_K, "K"},
            {Key::KEY_L, "L"},
            {Key::KEY_M, "M"},
            {Key::KEY_N, "N"},
            {Key::KEY_O, "O"},
            {Key::KEY_P, "P"},
            {Key::KEY_Q, "Q"},
            {Key::KEY_R, "R"},
            {Key::KEY_S, "S"},
            {Key::KEY_T, "T"},
            {Key::KEY_U, "U"},
            {Key::KEY_V, "V"},
            {Key::KEY_W, "W"},
            {Key::KEY_X, "X"},
            {Key::KEY_Y, "Y"},
            {Key::KEY_Z, "Z"},
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
        const auto key_id = static_cast<std::size_t>(key);

        if (key_id < KEY_COUNT)
            return KEY_TABLE[key_id].second;
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

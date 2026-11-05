/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Actions.hpp
*/

#ifndef R_TYPE_ACTIONS_HPP
    #define R_TYPE_ACTIONS_HPP
    #include <cstdint>
    #include <string_view>
    #include "engine/input/ActionMap.hpp"

namespace rtype::client {

    enum class Action : std::uint8_t {
        MOVE_UP, MOVE_DOWN, MOVE_LEFT, MOVE_RIGHT, FIRE, TOGGLE_HUD_HELP, QUIT, CONFIRM,
    };
    [[nodiscard]] std::string_view toString(Action action) noexcept;
    [[nodiscard]] bool fromString(std::string_view name, Action &action) noexcept;
    void bindDefaults(engine::input::ActionMap<Action> &actions);

    struct FrameContext {
        engine::input::InputState &input;
        engine::input::ActionMap<Action> &actions;
        engine::audio::IAudio &audio;
        engine::render::TextureCache &textures;
        engine::render::FontCache &fonts;
        const SpriteCatalog &sprites;
        const ClientConfig &config;
        const ClientSettings &settings;
        TimePoint now;
    };

}

#endif

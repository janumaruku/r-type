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

void bindDefaults(engine::input::ActionMap<Action> & actions);

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

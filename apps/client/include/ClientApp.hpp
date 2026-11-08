/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ClientApp.hpp
*/

#ifndef R_TYPE_CLIENTAPP_HPP
#define R_TYPE_CLIENTAPP_HPP

#include <memory>

#include "Actions.hpp"
#include "ClientConfig.hpp"
#include "ClientSettings.hpp"
#include "engine/audio/IAudio.hpp"
#include "engine/input/ActionMap.hpp"
#include "engine/input/InputState.hpp"
#include "engine/render/FontCache.hpp"
#include "engine/render/RaylibRenderer.hpp"
#include "engine/render/TextureCache.hpp"
#include "engine/render/Window.hpp"
#include "engine/runtime/StopWatch.hpp"
#include "Scenes.hpp"
#include "SoundBank.hpp"
#include "SpriteCatalog.hpp"

namespace rtype::client {

class ClientApp {
public:
    ClientApp(ClientConfig config, ClientSettings settings);

    [[nodiscard]] int run();

private:
    [[nodiscard]] FrameContext frameContext();

    ClientConfig _config;
    ClientSettings _settings;
    engine::render::Window _window;
    engine::render::TextureCache _textures;
    engine::render::FontCache _fonts;
    SpriteCatalog _sprites;
    engine::render::TextureId _whitePixel;
    engine::render::RaylibRenderer _renderer;
    std::unique_ptr<engine::audio::IAudio> _audio;
    SoundBank _sounds;
    engine::input::InputState _input;
    engine::input::ActionMap<Action> _actions;
    engine::render::RenderQueue _queue;
    SceneManager _scenes;
    engine::runtime::Stopwatch _frameClock;
};

} // namespace rtype::client

#endif

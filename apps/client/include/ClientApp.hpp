/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ClientApp.hpp
*/

#ifndef R_TYPE_CLIENTAPP_HPP
#define R_TYPE_CLIENTAPP_HPP
#include <memory>
#include "engine/render/include/RaylibRenderer.hpp"
#include "engine/render/include/RenderQueue.hpp"
#include "engine/input/include/engine/input/ActionMap.hpp"
// #include "engine/input/include/"

namespace rtype::client {

    class ClientApp {
    public:
        ClientApp(ClientConfig config, ClientSettings settings);
        [[nodiscard]] int run();

    private:
        ClientConfig _config;
        ClientSettings _settings;
        engine::render::Window _window;
        engine::render::TextureCache _textures;
        engine::render::FontCache _fonts;
        SpriteCatalog _sprites;
        engine::render::RaylibRenderer _renderer;
        std::unique_ptr<engine::audio::IAudio> _audio;
        engine::input::InputState _input;
        engine::input::ActionMap<Action> _actions;
        engine::render::RenderQueue _queue;
        SceneManager _scenes;
        engine::runtime::Stopwatch _frameClock;
    };

}

#endif

/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ClientApp
*/

#include "ClientApp.hpp"

#include <memory>
#include <utility>

#include "engine/audio/MakeAudio.hpp"
#include "engine/core/AssetLocator.hpp"
#include "engine/core/Logger.hpp"
#include "engine/runtime/Clock.hpp"

namespace rtype::client {
namespace {
const engine::core::Vec2u WINDOW_SIZE{1280, 720};
const engine::core::Vec2f LOGICAL_SIZE{1280.0F, 720.0F};
constexpr const char *CATALOG_FILE = "sprites/catalog.json";
constexpr const char *KEYBINDINGS_FILE = "config/keybindings.json";

engine::core::log::Logger &logger()
{
    static engine::core::log::Logger instance("client");

    return instance;
}

[[nodiscard]] engine::render::WindowConfig windowConfig(
    const ClientConfig &config, const ClientSettings &settings)
{
    return {
        .title = "R-Type",
        .size = WINDOW_SIZE,
        .fullscreen = config.fullscreen || settings.fullscreen,
        .vsync = true,
        .frameLimit = 0,
    };
}

/*
** Throws a ResourceError when the assets directory, the catalog or a sheet
** is missing, and a CatalogError when the catalog is broken.
*/
[[nodiscard]] SpriteCatalog loadSprites(engine::render::TextureCache &textures)
{
    const engine::core::AssetLocator assets;
    SpriteCatalog sprites;

    sprites.load(assets.resolve(CATALOG_FILE), textures);
    return sprites;
}

[[nodiscard]] std::unique_ptr<engine::audio::IAudio> startAudio(
    const ClientSettings &settings)
{
    auto audio = engine::audio::makeAudio();

    audio->setMasterVolume(settings.masterVolume);
    audio->setMusicVolume(settings.musicVolume);
    audio->setSfxVolume(settings.sfxVolume);
    audio->setMuted(settings.muted);
    return audio;
}

[[nodiscard]] engine::input::ActionMap<Action> loadBindings()
{
    engine::input::ActionMap<Action> actions;

    bindDefaults(actions);
    if (!actions.load(KEYBINDINGS_FILE))
        logger().warn("{} not loaded, using the default bindings",
            KEYBINDINGS_FILE);
    return actions;
}
}

/*
** TODO(RTP-117): start on a ConnectingScene once story 2 is done.
*/
ClientApp::ClientApp(ClientConfig config, ClientSettings settings):
    _config(std::move(config)), _settings(settings),
    _window(windowConfig(_config, _settings)),
    _sprites(loadSprites(_textures)),
    _renderer(_window, _textures, _fonts, LOGICAL_SIZE),
    _audio(startAudio(_settings)),
    _actions(loadBindings())
{
    FrameContext context = frameContext();

    _scenes.change(std::make_unique<MessageScene>("R-Type",
        "Not connected yet", false), context);
}

FrameContext ClientApp::frameContext()
{
    return {
        .input = _input,
        .actions = _actions,
        .audio = *_audio,
        .textures = _textures,
        .fonts = _fonts,
        .sprites = _sprites,
        .config = _config,
        .settings = _settings,
        .now = engine::runtime::Clock::now(),
    };
}
}

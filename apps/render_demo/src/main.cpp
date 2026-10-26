/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** main
*/

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "Actions.hpp"
#include "engine/audio/IAudio.hpp"
#include "engine/audio/MakeAudio.hpp"
#include "engine/audio/NullAudio.hpp"
#include "engine/core/AssetLocator.hpp"
#include "engine/core/Color.hpp"
#include "engine/core/Errors.hpp"
#include "FontCache.hpp"
#include "engine/core/Logger.hpp"
#include "RaylibRenderer.hpp"
#include "RenderQueue.hpp"
#include "Scene.hpp"
#include "TextureCache.hpp"
#include "Window.hpp"
#include "engine/input/InputEvent.hpp"
#include "engine/input/InputState.hpp"

namespace {
using engine::core::Vec2f;
using engine::input::InputEvent;
using engine::input::InputState;
using render_demo::Action;
using render_demo::CELL;
using render_demo::LOGICAL_SIZE;
using render_demo::Ship;
using Bindings = engine::input::ActionMap<Action>;

constexpr int EXIT_ERROR = 84;
constexpr float SHIP_SPEED = 400.0F;
constexpr float BULLET_SPEED = 900.0F;
constexpr float MAX_FRAME_TIME = 0.05F;
constexpr float VOLUME_STEP = 0.1F;
constexpr int EXPLOSIONS_PER_PRESS = 10;
const engine::core::Color BACKGROUND{12, 18, 40, 255};

/*
** Feeds the frame's events to the input state. Raylib reports no key
** releases while the window is unfocused, so losing focus releases every
** key: a held arrow can't keep the ship moving. Returns false once the
** window asks to close.
*/
bool handleEvents(const std::vector<InputEvent> &events, InputState &input,
    engine::core::log::Logger &logger)
{
    for (const InputEvent &event: events) {
        input.apply(event);
        switch (event.type) {
            case InputEvent::Type::WINDOW_CLOSED:
                return false;
            case InputEvent::Type::RESIZED:
                logger.info("RESIZED to {}x{}", event.size.x(),
                    event.size.y());
                break;
            case InputEvent::Type::FOCUS_LOST:
                input.releaseAll();
                logger.info("FOCUS_LOST");
                break;
            case InputEvent::Type::FOCUS_GAINED:
                logger.info("FOCUS_GAINED");
                break;
            default:
                break;
        }
    }
    return true;
}

float axis(const Bindings &bindings, const InputState &input,
    const Action negative, const Action positive)
{
    return static_cast<float>(bindings.isActive(positive, input))
        - static_cast<float>(bindings.isActive(negative, input));
}

void moveShip(Ship &ship, const Bindings &bindings, const InputState &input,
    const float dt)
{
    const Vec2f direction{
        axis(bindings, input, Action::MOVE_LEFT, Action::MOVE_RIGHT),
        axis(bindings, input, Action::MOVE_UP, Action::MOVE_DOWN)};
    const Vec2f moved = ship.position + direction * (SHIP_SPEED * dt);
    const float margin = CELL * render_demo::SPRITE_SCALE / 2.0F;

    ship.position = Vec2f{
        std::clamp(moved.x(), margin, LOGICAL_SIZE.x() - margin),
        std::clamp(moved.y(), margin, LOGICAL_SIZE.y() - margin)};
    ship.thrusting = direction != Vec2f{0.0F, 0.0F};
    if (direction.x() != 0.0F)
        ship.facingLeft = direction.x() < 0.0F;
}

void moveBullets(std::vector<Vec2f> &bullets, const float dt)
{
    for (Vec2f &bullet: bullets)
        bullet += Vec2f{BULLET_SPEED * dt, 0.0F};
    std::erase_if(bullets, [](const Vec2f &bullet) {
        return bullet.x() > LOGICAL_SIZE.x() + CELL;
    });
}

Bindings loadBindings(const engine::core::AssetLocator &assets,
    engine::core::log::Logger &logger)
{
    Bindings bindings = render_demo::defaultBindings();
    const std::filesystem::path file = assets.root() / "keybindings.json";

    if (bindings.load(file))
        logger.info("bindings loaded from {}", file.string());
    else
        logger.warn("{} not loaded, keeping the default bindings",
            file.string());
    return bindings;
}

/*
** The cache keys files by their normalised path, so a second load through a
** different spelling must hand back the first id instead of a new texture.
*/
void checkTextureIdsAreShared(engine::render::TextureCache &textures,
    const engine::core::AssetLocator &assets,
    const engine::render::TextureId sheet, engine::core::log::Logger &logger)
{
    const engine::render::TextureId again =
        textures.load(assets.root() / "." / "sprites.png");

    if (again != sheet)
        throw engine::core::EngineError("sprites.png loaded twice gave two ids");
    logger.info("sprites.png loaded twice: same id {}", sheet.value);
}

/*
** --missing-asset skips AssetLocator::resolve on purpose, so the error comes
** from the cache's own check: the ResourceError names the file, main logs
** it, and every destructor runs before the exit.
*/
void loadMissingAsset(engine::render::TextureCache &textures,
    const engine::core::AssetLocator &assets)
{
    (void)textures.load(assets.root() / "missing.png");
}

struct DemoSounds {
    engine::audio::SoundId shot;
    engine::audio::SoundId explosion;
    engine::audio::MusicId music;
};

/*
** The client config will start at master 1.0, sfx 0.6, music 0.8; the demo
** uses the same values so it sounds like the game.
*/
DemoSounds startAudio(engine::audio::IAudio &audio,
    const engine::core::AssetLocator &assets, engine::core::log::Logger &logger)
{
    const bool silent =
        dynamic_cast<engine::audio::NullAudio *>(&audio) != nullptr;
    const DemoSounds sounds{
        .shot = audio.loadSound(assets.resolve("shot.wav")),
        .explosion = audio.loadSound(assets.resolve("explosion.wav")),
        .music = audio.loadMusic(assets.resolve("music.ogg")),
    };

    logger.info("audio: {}", std::string{silent ? "NullAudio (silent)" : "RaylibAudio"});
    audio.setMasterVolume(1.0F);
    audio.setSfxVolume(0.6F);
    audio.setMusicVolume(0.8F);
    audio.playMusic(sounds.music, true);
    return sounds;
}

void handleAudioKeys(engine::audio::IAudio &audio, const DemoSounds &sounds,
    const Bindings &bindings, const InputState &input,
    engine::core::log::Logger &logger)
{
    if (bindings.wasTriggered(Action::EXPLODE, input)) {
        for (int i = 0; i < EXPLOSIONS_PER_PRESS; ++i)
            audio.playSound(sounds.explosion, 1.0F);
        logger.info("{} explosions in one frame", EXPLOSIONS_PER_PRESS);
    }
    if (bindings.wasTriggered(Action::VOLUME_UP, input))
        audio.setMasterVolume(audio.volumes().master() + VOLUME_STEP);
    if (bindings.wasTriggered(Action::VOLUME_DOWN, input))
        audio.setMasterVolume(audio.volumes().master() - VOLUME_STEP);
    if (bindings.wasTriggered(Action::MUTE, input))
        audio.setMuted(!audio.volumes().muted());
}

/*
** The order matters: the window first, then the caches and the renderer,
** which hold GPU resources and must be destroyed before the window closes.
** Each frame runs in the order the client will use: events, input state,
** game update, then fill, sort and submit the queue. Audio doesn't need the
** window; it comes after it only so Raylib's audio messages go through our
** logger. Its update() runs once per frame, on this thread.
*/
void run(engine::core::log::Logger &logger, const bool missingAsset)
{
    const engine::core::AssetLocator assets{
        std::filesystem::path{RENDER_DEMO_ASSETS}};
    engine::render::Window window{engine::render::WindowConfig{
        .title = "render_demo",
        .size = {1280, 720},
        .fullscreen = false,
        .vsync = true,
        .frameLimit = 60,
    }};
    engine::render::TextureCache textures;
    engine::render::FontCache fonts;
    engine::render::RaylibRenderer renderer{window, textures, fonts,
        LOGICAL_SIZE};
    const std::unique_ptr<engine::audio::IAudio> audio =
        engine::audio::makeAudio();
    const render_demo::SceneAssets scene{
        .sheet = textures.load(assets.resolve("sprites.png")),
        .font = fonts.load(assets.resolve("DejaVuSansMono.ttf")),
    };
    checkTextureIdsAreShared(textures, assets, scene.sheet, logger);
    if (missingAsset)
        loadMissingAsset(textures, assets);
    const Bindings bindings = loadBindings(assets, logger);
    const DemoSounds sounds = startAudio(*audio, assets, logger);
    InputState input;
    engine::render::RenderQueue queue;
    Ship ship{.position = LOGICAL_SIZE * 0.5F, .facingLeft = false,
        .thrusting = false};
    std::vector<Vec2f> bullets;
    unsigned fired = 0;
    auto previous = std::chrono::steady_clock::now();

    while (window.isOpen()) {
        input.beginFrame();
        if (!handleEvents(window.pollEvents(), input, logger))
            break;
        if (bindings.wasTriggered(Action::QUIT, input))
            break;
        if (bindings.wasTriggered(Action::FIRE, input)) {
            logger.info("FIRE #{}", ++fired);
            bullets.push_back(ship.position + Vec2f{CELL, 0.0F});
            audio->playSound(sounds.shot, 1.0F);
        }
        handleAudioKeys(*audio, sounds, bindings, input, logger);
        audio->update();

        const auto now = std::chrono::steady_clock::now();
        const float dt = std::min(
            std::chrono::duration<float>(now - previous).count(),
            MAX_FRAME_TIME);
        previous = now;
        moveShip(ship, bindings, input, dt);
        moveBullets(bullets, dt);

        queue.clear();
        render_demo::fillQueue(queue, scene, render_demo::Frame{
            .ship = ship, .bullets = bullets, .fired = fired,
            .volumes = audio->volumes()});
        queue.sortByLayer();
        renderer.beginFrame(BACKGROUND);
        renderer.submit(queue);
        renderer.endFrame();
    }
    logger.info("quit after {} shots", fired);
}
}

int main(int argc, char **argv)
{
    engine::core::log::Logger logger{"render_demo"};
    const bool missingAsset =
        argc == 2 && std::string_view{argv[1]} == "--missing-asset";

    if (argc > 2 || (argc == 2 && !missingAsset)) {
        logger.error("usage: {} [--missing-asset]", std::string{argv[0]});
        return EXIT_ERROR;
    }
    try {
        run(logger, missingAsset);
    } catch (const engine::core::EngineError &error) {
        logger.error("{}", std::string{error.what()});
        return EXIT_ERROR;
    }
    return 0;
}

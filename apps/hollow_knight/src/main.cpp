/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Hollow Knight demo application
*/

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <system_error>
#include <vector>

#include "Actions.hpp"
#include "engine/audio/IAudio.hpp"
#include "engine/audio/MakeAudio.hpp"
#include "engine/audio/NullAudio.hpp"
#include "engine/core/AssetLocator.hpp"
#include "engine/core/Errors.hpp"
#include "engine/core/Logger.hpp"
#include "engine/input/InputEvent.hpp"
#include "engine/input/InputState.hpp"
#include "engine/render/FontCache.hpp"
#include "engine/render/RaylibRenderer.hpp"
#include "engine/render/RenderQueue.hpp"
#include "engine/render/TextureCache.hpp"
#include "engine/render/Window.hpp"
#include "engine/runtime/FixedTimestep.hpp"
#include "Scene.hpp"
#include "World.hpp"

namespace {
using hollow_knight::Action;
using hollow_knight::Screen;
using Bindings = engine::input::ActionMap<Action>;

constexpr int EXIT_ERROR       = 84;
constexpr float MAX_FRAME_TIME = 0.05F;
const engine::core::Vec2f LOGICAL_SIZE{1280.0F, 720.0F};
const engine::core::Color BACKGROUND{5, 10, 21, 255};

bool handleEvents(const std::vector<engine::input::InputEvent> &events,
    engine::input::InputState &input)
{
    for (const engine::input::InputEvent &event: events) {
        input.apply(event);
        if (event.type == engine::input::InputEvent::Type::WINDOW_CLOSED)
            return false;
        if (event.type == engine::input::InputEvent::Type::FOCUS_LOST)
            input.releaseAll();
    }
    return true;
}

void moveSelection(const Bindings &actions,
    const engine::input::InputState &input, unsigned &selection,
    const unsigned count)
{
    if (actions.wasTriggered(Action::MENU_UP, input))
        selection = (selection + count - 1) % count;
    if (actions.wasTriggered(Action::MENU_DOWN, input))
        selection = (selection + 1) % count;
}

hollow_knight::PlayerInput readPlayerInput(const Bindings &actions,
    const engine::input::InputState &input)
{
    return hollow_knight::PlayerInput{
        .horizontal =
            static_cast<float>(actions.isActive(Action::MOVE_RIGHT, input)) -
            static_cast<float>(actions.isActive(Action::MOVE_LEFT, input)),
        .nailDirection = actions.isActive(Action::MENU_UP, input)
            ? hollow_knight::NailDirection::UP
            : actions.isActive(Action::MENU_DOWN, input)
            ? hollow_knight::NailDirection::DOWN
            : hollow_knight::NailDirection::HORIZONTAL,
        .jumpPressed   = actions.wasTriggered(Action::JUMP, input),
        .attackPressed = actions.wasTriggered(Action::ATTACK, input) ||
            input.wasPressed(engine::input::MouseButton::LEFT),
        .dashPressed     = actions.wasTriggered(Action::DASH, input),
        .focusHeld       = actions.isActive(Action::FOCUS, input),
        .interactPressed = actions.wasTriggered(Action::INTERACT, input),
    };
}

void updateTitle(Screen &screen, unsigned &selection, bool &running,
    const Bindings &actions, const engine::input::InputState &input,
    hollow_knight::World &world)
{
    moveSelection(actions, input, selection, 3);
    if (actions.wasTriggered(Action::PAUSE, input)) {
        running = false;
        return;
    }
    if (!actions.wasTriggered(Action::CONFIRM, input))
        return;

    if (selection == 0) {
        world.restart();
        screen    = Screen::PLAY;
        selection = 0;
    } else if (selection == 1) {
        screen = Screen::OPTIONS;
    } else {
        running = false;
    }
}

void updatePause(Screen &screen, unsigned &selection, bool &running,
    const Bindings &actions, const engine::input::InputState &input,
    hollow_knight::World &world)
{
    moveSelection(actions, input, selection, 3);
    if (actions.wasTriggered(Action::PAUSE, input)) {
        screen = Screen::PLAY;
        return;
    }
    if (!actions.wasTriggered(Action::CONFIRM, input))
        return;

    if (selection == 0) {
        screen    = Screen::PLAY;
        selection = 0;
    } else if (selection == 1) {
        screen    = Screen::TITLE;
        selection = 0;
        world.restart();
    } else {
        running = false;
    }
}

void run(engine::core::log::Logger &logger)
{
    const engine::core::AssetLocator assets;
    engine::render::Window window{engine::render::WindowConfig{
        .title      = "Hollow Knight - Forgotten Crossroads",
        .size       = {1280, 720},
        .fullscreen = false,
        .vsync      = true,
        .frameLimit = 60}};
    engine::render::TextureCache textures;
    engine::render::FontCache fonts;
    engine::render::RaylibRenderer renderer{
        window, textures, fonts, LOGICAL_SIZE};
    const std::unique_ptr<engine::audio::IAudio> audio =
        engine::audio::makeAudio();
    audio->setMusicVolume(0.68F);
    std::optional<engine::audio::MusicId> music;
    static constexpr std::array<const char *, 3> MUSIC_FILES{
        "greenpath.ogg", "greenpath.mp3", "greenpath.flac"};
    std::filesystem::path musicPath;
    bool isGreenpathMusic = false;
    std::error_code musicError;
    for (const char *file: MUSIC_FILES) {
        const auto candidate = assets.root() / "hollow_knight" / file;
        if (std::filesystem::is_regular_file(candidate, musicError)) {
            musicPath        = candidate;
            isGreenpathMusic = true;
            break;
        }
    }
    if (musicPath.empty())
        musicPath = assets.resolve("hollow_knight/ambient.ogg");
    music = audio->loadMusic(musicPath);
    audio->playMusic(*music, true);
    const engine::audio::SoundId nailSwing =
        audio->loadSound(assets.resolve("hollow_knight/nail_swing.wav"));
    const engine::audio::SoundId nailHit =
        audio->loadSound(assets.resolve("hollow_knight/nail_hit.wav"));
    audio->setSfxVolume(0.78F);
    const bool silent =
        dynamic_cast<engine::audio::NullAudio *>(audio.get()) != nullptr;
    logger.info("{} music: {}",
        std::string{isGreenpathMusic ? "Greenpath OST" : "Ambient fallback"},
        std::string{silent ? "loaded (audio device unavailable)" : "playing"});
    const auto loadSprite = [&textures, &assets](const char *path) {
        const engine::render::TextureId texture =
            textures.load(assets.resolve(path));
        const engine::core::Vec2u size = textures.size(texture);
        return hollow_knight::SpriteAsset{
            .texture = texture, .width = size.x(), .height = size.y()};
    };
    const hollow_knight::SceneAssets sceneAssets{
        .knightFrames   = {loadSprite("hollow_knight/knight_frame_0.png"),
            loadSprite("hollow_knight/knight_frame_1.png"),
            loadSprite("hollow_knight/knight_frame_2.png")},
        .nailSlash      = loadSprite("hollow_knight/nail_slash.png"),
        .healthMask     = loadSprite("hollow_knight/mask.png"),
        .enemies        = {loadSprite("hollow_knight/crawler.png"),
            loadSprite("hollow_knight/infected_husk.png"),
            loadSprite("hollow_knight/crossroads_guard.png")},
        .platformLedge  = loadSprite("hollow_knight/platform_ledge.png"),
        .platformStone  = loadSprite("hollow_knight/platform_stone.png"),
        .blueGrassSmall = loadSprite("hollow_knight/blue_grass_small.png"),
        .blueGrassTall  = loadSprite("hollow_knight/blue_grass_tall.png"),
        .rootPlatform   = loadSprite("hollow_knight/root_platform.png"),
        .ceilingThorns  = loadSprite("hollow_knight/ceiling_thorns.png"),
        .blueVine       = loadSprite("hollow_knight/blue_vine.png")};
    engine::render::RenderQueue queue;
    engine::input::InputState input;
    Bindings actions;
    hollow_knight::bindDefaults(actions);
    hollow_knight::World world;
    Screen screen      = Screen::TITLE;
    unsigned selection = 0;
    bool running       = true;
    float elapsed      = 0.0F;
    engine::runtime::FixedTimestep simulation{1.0 / 60.0, 5};
    hollow_knight::PlayerInput pendingInput;
    auto previous = std::chrono::steady_clock::now();

    logger.info("Hollow Knight room demo started");
    while (window.isOpen() && running) {
        input.beginFrame();
        if (!handleEvents(window.pollEvents(), input))
            break;
        audio->update();

        const auto now = std::chrono::steady_clock::now();
        const float dt =
            std::min(std::chrono::duration<float>(now - previous).count(),
                MAX_FRAME_TIME);
        previous = now;
        elapsed += dt;

        switch (screen) {
        case Screen::TITLE:
            updateTitle(screen, selection, running, actions, input, world);
            break;
        case Screen::OPTIONS:
            if (actions.wasTriggered(Action::PAUSE, input) ||
                actions.wasTriggered(Action::CONFIRM, input))
                screen = Screen::TITLE;
            break;
        case Screen::PLAY: {
            if (actions.wasTriggered(Action::PAUSE, input)) {
                screen    = Screen::PAUSE;
                selection = 0;
                break;
            }
            const hollow_knight::PlayerInput frameInput =
                readPlayerInput(actions, input);
            pendingInput.horizontal = frameInput.horizontal;
            if (frameInput.attackPressed)
                pendingInput.nailDirection = frameInput.nailDirection;
            pendingInput.focusHeld = frameInput.focusHeld;
            pendingInput.jumpPressed =
                pendingInput.jumpPressed || frameInput.jumpPressed;
            pendingInput.attackPressed =
                pendingInput.attackPressed || frameInput.attackPressed;
            pendingInput.dashPressed =
                pendingInput.dashPressed || frameInput.dashPressed;
            pendingInput.interactPressed =
                pendingInput.interactPressed || frameInput.interactPressed;
            const int steps = simulation.advance(dt);
            for (int step = 0; step < steps; ++step) {
                const std::uint32_t previousAttack =
                    world.player().attackSerial;
                const std::uint32_t previousHit = world.player().hitSerial;
                world.update(pendingInput,
                    static_cast<float>(simulation.step()));
                if (world.player().attackSerial != previousAttack)
                    audio->playSound(nailSwing, 0.76F);
                if (world.player().hitSerial != previousHit)
                    audio->playSound(nailHit, 0.88F);
                pendingInput.jumpPressed     = false;
                pendingInput.attackPressed   = false;
                pendingInput.dashPressed     = false;
                pendingInput.interactPressed = false;
            }
            break;
        }
        case Screen::PAUSE:
            updatePause(screen, selection, running, actions, input, world);
            break;
        }

        queue.clear();
        hollow_knight::fillScene(queue, screen, selection, elapsed, sceneAssets,
            world);
        queue.sortByLayer();
        renderer.beginFrame(BACKGROUND);
        renderer.submit(queue);
        renderer.endFrame();
    }
    logger.info("Hollow Knight room demo closed");
}
} // namespace

int main()
{
    engine::core::log::Logger logger{"hollow_knight"};
    try {
        run(logger);
    } catch (const engine::core::EngineError &error) {
        logger.error("{}", std::string{error.what()});
        return EXIT_ERROR;
    }
    return 0;
}

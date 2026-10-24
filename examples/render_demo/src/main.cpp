/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** main
*/

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <vector>

#include "Actions.hpp"
#include "AssetLocator.hpp"
#include "Color.hpp"
#include "Errors.hpp"
#include "FontCache.hpp"
#include "Logger.hpp"
#include "RaylibRenderer.hpp"
#include "RenderQueue.hpp"
#include "TextureCache.hpp"
#include "Window.hpp"
#include "engine/input/InputEvent.hpp"
#include "engine/input/InputState.hpp"

namespace {
using engine::core::Vec2f;
using engine::input::InputEvent;
using engine::input::InputState;
using render_demo::Action;
using Bindings = engine::input::ActionMap<Action>;

constexpr int EXIT_ERROR = 84;
constexpr int CELL = 32;
constexpr float SHIP_SCALE = 2.0F;
constexpr float SHIP_SPEED = 400.0F;
constexpr float MAX_FRAME_TIME = 0.05F;
constexpr std::int16_t SHIP_LAYER = 500;
const Vec2f LOGICAL_SIZE{1280.0F, 720.0F};
const engine::core::Color BACKGROUND{12, 18, 40, 255};

struct Ship {
    Vec2f position;
    bool facingLeft;
    bool thrusting;
};

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
    const float margin = CELL * SHIP_SCALE / 2.0F;

    ship.position = Vec2f{
        std::clamp(moved.x(), margin, LOGICAL_SIZE.x() - margin),
        std::clamp(moved.y(), margin, LOGICAL_SIZE.y() - margin)};
    ship.thrusting = direction != Vec2f{0.0F, 0.0F};
    if (direction.x() != 0.0F)
        ship.facingLeft = direction.x() < 0.0F;
}

/*
** Cell 0 is the ship, cell 1 the ship with its flame. A negative source
** width turns it to face left.
*/
void drawShip(engine::render::RenderQueue &queue,
    const engine::render::TextureId sheet, const Ship &ship)
{
    const int cell = ship.thrusting ? 1 : 0;

    queue.push(engine::render::SpriteCmd{
        .texture = sheet,
        .source = engine::core::IntRect{engine::core::Vec2i{cell * CELL, 0},
            ship.facingLeft ? -CELL : CELL, CELL},
        .position = ship.position,
        .origin = Vec2f{CELL / 2.0F, CELL / 2.0F},
        .scale = Vec2f{SHIP_SCALE, SHIP_SCALE},
        .rotationDeg = 0.0F,
        .tint = engine::core::Color{255, 255, 255, 255},
        .layer = SHIP_LAYER,
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
** The order matters: the window first, then the caches and the renderer,
** which hold GPU resources and must be destroyed before the window closes.
** Each frame runs in the order the client will use: events, input state,
** game update, then fill, sort and submit the queue.
*/
void run(engine::core::log::Logger &logger)
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
    const engine::render::TextureId sheet =
        textures.load(assets.resolve("sprites.png"));
    const Bindings bindings = loadBindings(assets, logger);
    InputState input;
    engine::render::RenderQueue queue;
    Ship ship{.position = LOGICAL_SIZE * 0.5F, .facingLeft = false,
        .thrusting = false};
    unsigned fired = 0;
    auto previous = std::chrono::steady_clock::now();

    while (window.isOpen()) {
        input.beginFrame();
        if (!handleEvents(window.pollEvents(), input, logger))
            break;
        if (bindings.wasTriggered(Action::QUIT, input))
            break;
        if (bindings.wasTriggered(Action::FIRE, input))
            logger.info("FIRE #{}", ++fired);

        const auto now = std::chrono::steady_clock::now();
        const float dt = std::min(
            std::chrono::duration<float>(now - previous).count(),
            MAX_FRAME_TIME);
        previous = now;
        moveShip(ship, bindings, input, dt);

        queue.clear();
        drawShip(queue, sheet, ship);
        queue.sortByLayer();
        renderer.beginFrame(BACKGROUND);
        renderer.submit(queue);
        renderer.endFrame();
    }
    logger.info("quit after {} shots", fired);
}
}

int main()
{
    engine::core::log::Logger logger{"render_demo"};

    try {
        run(logger);
    } catch (const engine::core::EngineError &error) {
        logger.error("{}", std::string{error.what()});
        return EXIT_ERROR;
    }
    return 0;
}

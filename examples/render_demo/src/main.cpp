/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** main
*/

#include <filesystem>
#include <vector>

#include "AssetLocator.hpp"
#include "Color.hpp"
#include "Errors.hpp"
#include "FontCache.hpp"
#include "Logger.hpp"
#include "RaylibRenderer.hpp"
#include "TextureCache.hpp"
#include "Window.hpp"
#include "engine/input/InputEvent.hpp"

namespace {
using engine::input::InputEvent;

constexpr int EXIT_ERROR = 84;
const engine::core::Vec2f LOGICAL_SIZE{1280.0F, 720.0F};
const engine::core::Color BACKGROUND{12, 18, 40, 255};

/*
** Returns false once the window asks to close.
*/
bool handleEvents(const std::vector<InputEvent> &events,
    engine::core::log::Logger &logger)
{
    for (const InputEvent &event: events) {
        switch (event.type) {
            case InputEvent::Type::WINDOW_CLOSED:
                return false;
            case InputEvent::Type::RESIZED:
                logger.info("RESIZED to {}x{}", event.size.x(),
                    event.size.y());
                break;
            case InputEvent::Type::FOCUS_LOST:
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

/*
** The order matters: the window first, then the caches and the renderer,
** which hold GPU resources and must be destroyed before the window closes.
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

    logger.info("assets from {}", assets.root().string());
    while (window.isOpen()) {
        if (!handleEvents(window.pollEvents(), logger))
            break;
        renderer.beginFrame(BACKGROUND);
        renderer.endFrame();
    }
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

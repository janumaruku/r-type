/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** main
*/

#include <algorithm>
#include <string>
#include <vector>

#include "FontCache.hpp"
#include "RaylibRenderer.hpp"
#include "RenderQueue.hpp"
#include "TextureCache.hpp"
#include "Window.hpp"
#include "engine/core/Color.hpp"
#include "engine/core/Errors.hpp"
#include "engine/core/Logger.hpp"
#include "engine/core/Vec2.hpp"
#include "engine/input/InputEvent.hpp"

namespace {
using engine::input::InputEvent;

constexpr int EXIT_ERROR = 84;
const engine::core::Vec2f LOGICAL_SIZE{1280.0F, 720.0F};
const engine::core::Color BACKGROUND{18, 18, 24, 255};

bool shouldQuit(const std::vector<InputEvent> &events)
{
    return std::ranges::any_of(events, [](const InputEvent &event) {
        return event.type == InputEvent::Type::WINDOW_CLOSED
            || (event.type == InputEvent::Type::KEY_PRESSED
                && event.key == engine::input::Key::ESCAPE);
    });
}

/**
 * @brief Opens the window and draws frames until it closes or Escape is
 * pressed.
 *
 * The caches and the renderer hold GPU resources, so they are created after
 * the window and destroyed before it. The queue stays empty until Kollection
 * has something to paint.
 */
void run()
{
    engine::render::Window window{engine::render::WindowConfig{
        .title = "kollection_demo",
        .size = {1280, 720},
        .fullscreen = false,
        .vsync = true,
        .frameLimit = 60,
    }};
    engine::render::TextureCache textures;
    engine::render::FontCache fonts;
    engine::render::RaylibRenderer renderer{window, textures, fonts,
        LOGICAL_SIZE};
    engine::render::RenderQueue queue;

    while (window.isOpen()) {
        if (shouldQuit(window.pollEvents()))
            break;
        queue.clear();
        renderer.beginFrame(BACKGROUND);
        renderer.submit(queue);
        renderer.endFrame();
    }
}
}

int main()
{
    engine::core::log::Logger logger{"kollection_demo"};

    try {
        run();
    } catch (const engine::core::EngineError &error) {
        logger.error("{}", std::string{error.what()});
        return EXIT_ERROR;
    }
    return 0;
}

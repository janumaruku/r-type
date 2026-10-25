/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Window.cpp
*/


#include <array>
#include <raylib.h>
#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <string>

#include "Window.hpp"
#include "engine/core/Errors.hpp"
#include "KeyTable.hpp"
#include "engine/core/Logger.hpp"

namespace engine::render {

struct Window::Impl {
    core::Vec2u size{};
    bool shouldClose = false;     // set by close()
    bool closeReported = false;   // WINDOW_CLOSED is sent once
    bool focused = true;          // Raylib sends no focus events, so we compare
};

namespace {

void raylibTraceCallback(int logLevel, const char *text, va_list args)
{
    static core::log::Logger raylibLog{"raylib"};

    std::array<char, 512> buffer;
    std::vsnprintf(buffer.data(), sizeof buffer, text, args);
    const std::string message{buffer.data()};

    switch (logLevel) {
        case LOG_TRACE:   raylibLog.trace() << message; break;
        case LOG_DEBUG:   raylibLog.debug() << message; break;
        case LOG_WARNING: raylibLog.warn()  << message; break;
        case LOG_ERROR:
        case LOG_FATAL:   raylibLog.error() << message; break;
        case LOG_INFO:
        default:          raylibLog.info()  << message; break;
    }
}

}  // namespace

Window::Window(WindowConfig const& config)
    : _impl(std::make_unique<Impl>())
{
    assert(!IsWindowReady() && "only one render::Window can exist at a time");

    SetTraceLogCallback(raylibTraceCallback);

    unsigned int flags = FLAG_WINDOW_RESIZABLE;
    if (config.vsync)
        flags |= FLAG_VSYNC_HINT;
    if (config.fullscreen)
        flags |= FLAG_FULLSCREEN_MODE;
    SetConfigFlags(flags);

    InitWindow(static_cast<int>(config.size.x()), static_cast<int>(config.size.y()),
               config.title.c_str());

    if (!IsWindowReady())
        throw core::EngineError("cannot open the window: no display, or no "
                                "working OpenGL driver");
    SetExitKey(KEY_NULL);
    if (config.frameLimit > 0)
        SetTargetFPS(static_cast<int>(config.frameLimit));

    _impl->size = {static_cast<unsigned>(GetScreenWidth()),
                   static_cast<unsigned>(GetScreenHeight())};
}

Window::~Window()
{
    if (IsWindowReady())
        CloseWindow();
}

bool Window::isOpen() const
{
    return IsWindowReady() && !shouldClose();
}

void Window::close()
{
    _impl->shouldClose = true;
}

bool Window::shouldClose() const
{
    return _impl->shouldClose || WindowShouldClose();
}

const core::Vec2u &Window::size() const
{
    return _impl->size;
}

void Window::setTitle(std::string_view title)
{
    SetWindowTitle(std::string(title).c_str());
}

std::vector<input::InputEvent> Window::pollEvents()
{
    using input::InputEvent;
    std::vector<InputEvent> events;

    for (std::size_t i = detail::keyIndex(input::Key::UNKNOWN) + 1; i < input::KEY_COUNT; ++i) {
        const auto key = static_cast<input::Key>(i);
        const int raylibKey = detail::KEY_TABLE[i];
        if (IsKeyPressed(raylibKey))
            events.push_back({.type=InputEvent::Type::KEY_PRESSED, .key=key, .size={}});
        if (IsKeyReleased(raylibKey))
            events.push_back({.type=InputEvent::Type::KEY_RELEASED, .key=key, .size={}});
    }

    // Window close button
    if (WindowShouldClose() && !_impl->closeReported) {
        _impl->closeReported = true;
        events.push_back({.type=InputEvent::Type::WINDOW_CLOSED, .key=input::Key::UNKNOWN, .size={}});
    }

    // Focus
    bool focused = IsWindowFocused();
    if (focused != _impl->focused) {
        _impl->focused = focused;
        events.push_back({.type=focused ? InputEvent::Type::FOCUS_GAINED
                                  : InputEvent::Type::FOCUS_LOST,
                          .key=input::Key::UNKNOWN, .size={}});
    }

    // Resize
    if (IsWindowResized()) {
        _impl->size = {static_cast<unsigned>(GetScreenWidth()),
                       static_cast<unsigned>(GetScreenHeight())};
        events.push_back({.type=InputEvent::Type::RESIZED, .key=input::Key::UNKNOWN, .size=_impl->size});
    }
    return events;
}

} // namespace engine::render
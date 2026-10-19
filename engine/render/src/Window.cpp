/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Window.cpp
*/


#include <array>
#include <raylib.h>
#include <cassert>
#include <stdexcept>
#include <cstdarg>
#include <cstdio>
#include <string>

#include "Window.hpp"
#include "Logger.hpp"

namespace engine::render {

struct Window::Impl {
    core::Vec2u size{};
    bool shouldClose = false;     // set by close()
    bool closeReported = false;   // WINDOW_CLOSED is sent once
    bool focused = true;          // Raylib sends no focus events, so we compare
};

namespace {

int toRaylibKey(input::Key key)
{
    switch (key) {
        case input::Key::UP:     return KEY_UP;
        case input::Key::DOWN:   return KEY_DOWN;
        case input::Key::LEFT:   return KEY_LEFT;
        case input::Key::RIGHT:  return KEY_RIGHT;
        case input::Key::SPACE:  return KEY_SPACE;
        case input::Key::ENTER:  return KEY_ENTER;
        case input::Key::ESCAPE: return KEY_ESCAPE;
        case input::Key::LSHIFT: return KEY_LEFT_SHIFT;
        case input::Key::LCTRL:  return KEY_LEFT_CONTROL;
        case input::Key::KEY_A:      return KEY_A;
        case input::Key::KEY_B:      return KEY_B;
        case input::Key::KEY_C:      return KEY_C;
        case input::Key::KEY_D:      return KEY_D;
        case input::Key::KEY_E:      return KEY_E;
        case input::Key::KEY_F:      return KEY_F;
        case input::Key::KEY_G:      return KEY_G;
        case input::Key::KEY_H:      return KEY_H;
        case input::Key::KEY_I:      return KEY_I;
        case input::Key::KEY_J:      return KEY_J;
        case input::Key::KEY_K:      return KEY_K;
        case input::Key::KEY_L:      return KEY_L;
        case input::Key::KEY_M:      return KEY_M;
        case input::Key::KEY_N:      return KEY_N;
        case input::Key::KEY_O:      return KEY_O;
        case input::Key::KEY_P:      return KEY_P;
        case input::Key::KEY_Q:      return KEY_Q;
        case input::Key::KEY_R:      return KEY_R;
        case input::Key::KEY_S:      return KEY_S;
        case input::Key::KEY_T:      return KEY_T;
        case input::Key::KEY_U:      return KEY_U;
        case input::Key::KEY_V:      return KEY_V;
        case input::Key::KEY_W:      return KEY_W;
        case input::Key::KEY_X:      return KEY_X;
        case input::Key::KEY_Y:      return KEY_Y;
        case input::Key::KEY_Z:      return KEY_Z;
        default:                 return KEY_NULL;
    }
}

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

    std::uint16_t flags = 0;
    if (config.vsync)
        flags |= FLAG_VSYNC_HINT;
    if (config.fullscreen)
        flags |= FLAG_FULLSCREEN_MODE;
    SetConfigFlags(flags);

    InitWindow(static_cast<int>(config.size.x()), static_cast<int>(config.size.y()),
               config.title.c_str());

    if (!IsWindowReady()) {
        core::log::Logger("window").error("Cannot open window");
        throw std::runtime_error("");
    }
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

    // Keys
    for (std::size_t i = 0; i < static_cast<std::size_t>(input::Key::COUNT); ++i) {
        auto key = static_cast<input::Key>(i);
        int raylibKey = toRaylibKey(key);
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
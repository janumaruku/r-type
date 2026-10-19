#include "Window.hpp"

#include <raylib.h>

#include <cassert>

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
        case input::Key::LALT:   return KEY_LEFT_ALT;
        case input::Key::RSHIFT: return KEY_RIGHT_SHIFT;
        case input::Key::RCTRL:  return KEY_RIGHT_CONTROL;
        case input::Key::RALT:   return KEY_RIGHT_ALT;
        case input::Key::A:      return KEY_A;
        case input::Key::B:      return KEY_B;
        case input::Key::C:      return KEY_C;
        case input::Key::D:      return KEY_D;
        case input::Key::E:      return KEY_E;
        case input::Key::F:      return KEY_F;
        case input::Key::G:      return KEY_G;
        case input::Key::H:      return KEY_H;
        case input::Key::I:      return KEY_I;
        case input::Key::J:      return KEY_J;
        case input::Key::K:      return KEY_K;
        case input::Key::L:      return KEY_L;
        case input::Key::M:      return KEY_M;
        case input::Key::N:      return KEY_N;
        case input::Key::O:      return KEY_O;
        case input::Key::P:      return KEY_P;
        case input::Key::Q:      return KEY_Q;
        case input::Key::R:      return KEY_R;
        case input::Key::S:      return KEY_S;
        case input::Key::T:      return KEY_T;
        case input::Key::U:      return KEY_U;
        case input::Key::V:      return KEY_V;
        case input::Key::W:      return KEY_W;
        case input::Key::X:      return KEY_X;
        case input::Key::Y:      return KEY_Y;
        case input::Key::Z:      return KEY_Z;
        default:                 return KEY_NULL;
    }
}

}  // namespace

Window::Window(WindowConfig const& config)
    : _impl(std::make_unique<Impl>())
{
    assert(!IsWindowReady() && "only one render::Window can exist at a time");

    unsigned flags = 0;
    if (config.vsync)
        flags |= FLAG_VSYNC_HINT;
    if (config.fullscreen)
        flags |= FLAG_FULLSCREEN_MODE;
    SetConfigFlags(flags);

    InitWindow(static_cast<int>(config.size.x()), static_cast<int>(config.size.y()),
               config.title.c_str());
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
            events.push_back({InputEvent::Type::KEY_PRESSED, key, {}});
        if (IsKeyReleased(raylibKey))
            events.push_back({InputEvent::Type::KEY_RELEASED, key, {}});
    }

    // Window close button
    if (WindowShouldClose() && !_impl->closeReported) {
        _impl->closeReported = true;
        events.push_back({InputEvent::Type::WINDOW_CLOSED, input::Key::UNKNOWN, {}});
    }

    // Focus
    bool focused = IsWindowFocused();
    if (focused != _impl->focused) {
        _impl->focused = focused;
        events.push_back({focused ? InputEvent::Type::FOCUS_GAINED
                                  : InputEvent::Type::FOCUS_LOST,
                          input::Key::UNKNOWN, {}});
    }

    // Resize
    if (IsWindowResized()) {
        _impl->size = {static_cast<unsigned>(GetScreenWidth()),
                       static_cast<unsigned>(GetScreenHeight())};
        events.push_back({InputEvent::Type::RESIZED, input::Key::UNKNOWN, _impl->size});
    }

    return events;
}

} // namespace engine::render
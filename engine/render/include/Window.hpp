/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Window.hpp
*/

#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <memory>
#include <string_view>
#include <vector>

#include "Vec2.hpp"

#include "engine/input/InputEvent.hpp"

namespace engine::render {

struct WindowConfig {
    std::string title;
    core::Vec2u size = {1280, 720};
    bool fullscreen = false;
    bool vsync = true;
    std::uint8_t frameLimit = 0;
};

class Window {
public:
    explicit Window(WindowConfig const& config);

    Window(Window &) = delete;
    Window(const Window &) = delete;
    Window(Window &&) =delete;
    Window(const Window &&) = delete;

    Window operator=(const Window &) = delete;
    Window operator=(const Window &&) = delete;

    ~Window();

    [[nodiscard]] bool isOpen() const;

    void close();

    [[nodiscard]] std::vector<engine::input::InputEvent> pollEvents();

    [[nodiscard]] bool shouldClose() const;

    [[nodiscard]] const core::Vec2u &size() const;

    void setTitle(std::string_view title);

private:
    struct Impl;
    std::unique_ptr<Impl> _impl;

    bool _shouldClose = false;
    bool _isOpen = false;
};

} // namespace engine::render

#endif // WINDOW_HPP
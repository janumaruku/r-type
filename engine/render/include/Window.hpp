/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Window.hpp
*/

#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <cstdint>
#include <memory>
#include <string>
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

    Window(const Window &) = delete;

    Window(Window &&) = delete;

    Window &operator=(const Window &) = delete;

    Window &operator=(Window &&) = delete;

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
};

} // namespace engine::render

#endif // WINDOW_HPP

/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Window.cpp
*/

#include "Window.hpp"


namespace engine::render {

Window::Window(WindowConfig const &config):
    _size(config.size), _title(config.title), _fullscreen(config.fullscreen),
    _vsync(config.vsync), _isOpen(true)
{
}

bool Window::isOpen() const
{
    return _isOpen;
}

void Window::close()
{
    _shouldClose = true;
    _isOpen = false;
}

bool Window::shouldClose() const
{
    return _shouldClose;
}

const core::Vec2u &Window::size() const
{
    return _size;
}

void Window::setTitle(std::string_view title)
{
    _title = title;
}

} // namespace engine::render

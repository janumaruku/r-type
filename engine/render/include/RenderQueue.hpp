/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** RenderQueue.hpp
*/

#ifndef RENDER_QUEUE_HPP
#define RENDER_QUEUE_HPP

#include <span>
#include <vector>

#include "DrawCommand.hpp"

namespace engine::render {
    
class RenderQueue {
public:
    RenderQueue() = default;

    void push(DrawCommand command);
    void clear();
    void sortByLayer();
    
    std::span<const DrawCommand> commands();
    std::size_t capacity() const;

private:
    std::vector<DrawCommand> _commands;
};

}   

#endif /* !RENDER_QUEUE_HPP */
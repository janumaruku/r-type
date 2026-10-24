/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** RenderQueue.cpp
*/

#include <algorithm>
#include <utility>
#include <variant>

#include "RenderQueue.hpp"
#include "DrawCommand.hpp"

namespace engine::render {

void RenderQueue::push(DrawCommand command)
{
    _commands.push_back(std::move(command));
}

void RenderQueue::clear()
{
    _commands.clear();
}

void RenderQueue::sortByLayer()
{
    std::ranges::stable_sort(_commands.begin(), _commands.end(),
        [](const DrawCommand &lhs, const DrawCommand &rhs) {
            const auto lhsLayer =
                std::visit([](const auto &command) { return command.layer; },
                    lhs);

            const auto rhsLayer =
                std::visit([](const auto &command) { return command.layer; },
                    rhs);

            return lhsLayer < rhsLayer;
        });
}

std::span<const DrawCommand> RenderQueue::commands() const
{
    return _commands;
}

std::size_t RenderQueue::capacity() const
{
    return _commands.capacity();
}

} // namespace engine::render

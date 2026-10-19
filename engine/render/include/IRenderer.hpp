/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** IRenderer.hpp
*/

#ifndef IRENDERER_HPP
#define IRENDERER_HPP

#include "DrawCommand.hpp"
#include "RenderQueue.hpp"

namespace engine::render {
class IRenderer {
public:
    virtual ~IRenderer() = default;
    virtual void render() = 0;
    virtual void beginFrame(Color clear) = 0;
    virtual void submit(const RenderQueue &queue) = 0;
};
}
#endif // ! IRENDERER_HPP
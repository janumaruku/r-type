/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** IRenderer.hpp
*/

#ifndef IRENDERER_HPP
#define IRENDERER_HPP

#include "engine/core/Color.hpp"
#include "RenderQueue.hpp"
#include "engine/core/Vec2.hpp"

namespace engine::render {

/*
** One frame is beginFrame, any number of submit calls, then endFrame.
** Commands are drawn in logical coordinates, in the order given: sorting is
** the caller's job (RenderQueue::sortByLayer).
*/
class IRenderer {
public:
    IRenderer() = default;

    IRenderer(const IRenderer &) = delete;

    IRenderer(IRenderer &&) = delete;

    virtual ~IRenderer() = default;

    IRenderer &operator=(const IRenderer &) = delete;

    IRenderer &operator=(IRenderer &&) = delete;

    virtual void beginFrame(core::Color clear) = 0;

    virtual void submit(const RenderQueue &queue) = 0;

    virtual void endFrame() = 0;

    virtual void setLogicalSize(core::Vec2f size) = 0;
};

}

#endif // !IRENDERER_HPP

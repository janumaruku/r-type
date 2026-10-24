/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** RaylibRenderer.hpp
*/

#ifndef RAYLIB_RENDERER_HPP
#define RAYLIB_RENDERER_HPP

#include <memory>

#include "FontCache.hpp"
#include "IRenderer.hpp"
#include "TextureCache.hpp"
#include "Window.hpp"

namespace engine::render {

/*
** Draws into an off-screen texture of the logical size, then letterboxes it
** into the window. It owns GPU state, so like the caches it must be created
** after the Window and destroyed before it.
*/
class RaylibRenderer final : public IRenderer {
public:
    RaylibRenderer(Window &window, TextureCache &textures, FontCache &fonts,
        core::Vec2f logicalSize);

    ~RaylibRenderer() override;

    void beginFrame(core::Color clear) override;

    void submit(const RenderQueue &queue) override;

    void endFrame() override;

    void setLogicalSize(core::Vec2f size) override;

private:
    struct Impl;
    std::unique_ptr<Impl> _impl;
};

}

#endif

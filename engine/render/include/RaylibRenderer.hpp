/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** RaylibRenderer.hpp
*/

#ifndef RAYLIB_RENDERER_HPP
#define RAYLIB_RENDERER_HPP

#include "IRenderer.hpp"
#include "Window.hpp"

namespace engine::render {

class RaylibRenderer final : public IRenderer {
public:
    RaylibRenderer();
    RaylibRenderer(const RaylibRenderer &) = delete;
    RaylibRenderer &operator=(const RaylibRenderer &) = delete;

    ~RaylibRenderer() override = default;
    void render() override;
    void beginFrame(Color clear) override;
    void submit(const RenderQueue &queue) override;
private:
    Window &_window;
    TextureCache &_textures;
    FontCache &_fonts;
    RenderTexture2D _target;
};
}

#endif
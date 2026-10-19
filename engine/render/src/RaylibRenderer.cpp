/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** RaylibRenderer.cpp
*/

#include "RaylibRenderer.hpp"

namespace engine::render {

RaylibRenderer::RaylibRenderer() : _window{Window::getInstance()}, _textures{TextureCache::getInstance()}, _fonts{FontCache::getInstance()}
{
    _target = LoadRenderTexture(_window.size().x(), _window.size().y());
}
void RaylibRenderer::render()
{
   beginFrame(BLANK);

}

void RaylibRenderer::beginFrame(Color clear)
{
    BeginTextureMode(_target);
    ClearBackground(clear);
}
} // namespace engine::render

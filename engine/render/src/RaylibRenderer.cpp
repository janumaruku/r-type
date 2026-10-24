/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** RaylibRenderer.cpp
*/

#include "RaylibRenderer.hpp"

#include <cassert>

#include "Errors.hpp"
#include "Letterbox.hpp"
#include "RaylibConvert.hpp"

#include <raylib.h>

namespace engine::render {

namespace {
constexpr ::Color BARS{0, 0, 0, 255};
constexpr ::Color NO_TINT{255, 255, 255, 255};

RenderTexture2D createTarget(const core::Vec2f &size)
{
    if (!IsWindowReady())
        throw core::EngineError("RaylibRenderer created before the window");

    const RenderTexture2D target = LoadRenderTexture(
        static_cast<int>(size.x()), static_cast<int>(size.y()));
    if (!IsRenderTextureValid(target))
        throw core::EngineError("cannot create the render texture");
    return target;
}
}

struct RaylibRenderer::Impl {
    Window &window;
    TextureCache &textures;
    FontCache &fonts;
    core::Vec2f logicalSize;
    RenderTexture2D target;
    bool inFrame = false;
};

RaylibRenderer::RaylibRenderer(Window &window, TextureCache &textures,
    FontCache &fonts, const core::Vec2f logicalSize)
    : _impl(std::make_unique<Impl>(Impl{.window = window,
        .textures = textures,
        .fonts = fonts,
        .logicalSize = logicalSize,
        .target = createTarget(logicalSize),
        .inFrame = false}))
{
}

RaylibRenderer::~RaylibRenderer()
{
    UnloadRenderTexture(_impl->target);
}

void RaylibRenderer::beginFrame(const core::Color clear)
{
    assert(!_impl->inFrame && "beginFrame called twice without endFrame");
    _impl->inFrame = true;
    BeginTextureMode(_impl->target);
    ClearBackground(detail::toRaylib(clear));
}

/*
** TODO(RTP-77): draw each command in order (step 6).
*/
void RaylibRenderer::submit([[maybe_unused]] const RenderQueue &queue)
{
    assert(_impl->inFrame && "submit called outside beginFrame/endFrame");
}

/*
** OpenGL stores render textures bottom-up, so the source rectangle has a
** negative height to draw the playfield the right way up. A minimised window
** gives an empty letterbox, and the texture isn't drawn at all.
*/
void RaylibRenderer::endFrame()
{
    assert(_impl->inFrame && "endFrame called without beginFrame");
    _impl->inFrame = false;
    EndTextureMode();

    const auto &texture = _impl->target.texture;
    const Rectangle source{0.0F, 0.0F, static_cast<float>(texture.width),
        -static_cast<float>(texture.height)};
    const core::FloatRect destination =
        letterbox(_impl->window.size(), _impl->logicalSize);

    BeginDrawing();
    ClearBackground(BARS);
    if (destination.width() > 0.0F && destination.height() > 0.0F)
        DrawTexturePro(texture, source, detail::toRaylib(destination),
            Vector2{0.0F, 0.0F}, 0.0F, NO_TINT);
    EndDrawing();
}

void RaylibRenderer::setLogicalSize(const core::Vec2f size)
{
    assert(!_impl->inFrame && "setLogicalSize called during a frame");
    const RenderTexture2D target = createTarget(size);

    UnloadRenderTexture(_impl->target);
    _impl->target = target;
    _impl->logicalSize = size;
}

}

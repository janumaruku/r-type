/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** RaylibRenderer.cpp
*/

#include "RaylibRenderer.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <variant>

#include "CacheAccess.hpp"
#include "engine/core/Errors.hpp"
#include "Letterbox.hpp"
#include "RaylibConvert.hpp"

#include <raylib.h>

namespace engine::render {

namespace {
constexpr ::Color BARS{.r = 0, .g = 0, .b = 0, .a = 255};
constexpr ::Color NO_TINT{.r = 255, .g = 255, .b = 255, .a = 255};

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

[[maybe_unused]] std::int16_t layerOf(const DrawCommand &command)
{
    return std::visit([](const auto &cmd) {
        return cmd.layer;
    }, command);
}

/*
** Destination size is the source size times the scale, and the origin is
** scaled the same way so rotation turns around the right point. A negative
** source width or height flips the sprite: Raylib reads the sign from the
** source, so the destination keeps the absolute size.
** The default font is 10 px, so it gets DrawText's spacing of size / 10;
** loaded fonts already space their glyphs.
*/
struct CommandDrawer {
    const TextureCache &textures;
    const FontCache &fonts;

    void operator()(const SpriteCmd &cmd) const
    {
        const Rectangle source = detail::toRaylib(cmd.source);
        const float width = std::fabs(source.width) * cmd.scale.x();
        const float height = std::fabs(source.height) * cmd.scale.y();
        const Vector2 origin{cmd.origin.x() * cmd.scale.x(),
                             cmd.origin.y() * cmd.scale.y()};

        DrawTexturePro(detail::CacheAccess::texture(textures, cmd.texture),
            source, Rectangle{cmd.position.x(), cmd.position.y(), width,
                              height}, origin, cmd.rotationDeg,
            detail::toRaylib(cmd.tint));
    }

    void operator()(const RectCmd &cmd) const
    {
        const Rectangle rect = detail::toRaylib(cmd.rect);

        DrawRectangleRec(rect, detail::toRaylib(cmd.fill));
        if (cmd.outlineThickness > 0.0F)
            DrawRectangleLinesEx(rect, cmd.outlineThickness,
                detail::toRaylib(cmd.outline));
    }

    void operator()(const TextCmd &cmd) const
    {
        const auto size = static_cast<float>(cmd.characterSize);
        const float spacing = cmd.font.isDefault() ? size / 10.0F : 0.0F;

        DrawTextEx(detail::CacheAccess::font(fonts, cmd.font),
            cmd.text.c_str(), detail::toRaylib(cmd.position), size, spacing,
            detail::toRaylib(cmd.color));
    }
};
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
** Commands are drawn in the order given; sorting is the caller's job, which
** debug builds check.
*/
void RaylibRenderer::submit(const RenderQueue &queue)
{
    assert(_impl->inFrame && "submit called outside beginFrame/endFrame");
    assert(std::ranges::is_sorted(queue.commands(), {}, layerOf)
        && "submit needs a queue sorted by layer (RenderQueue::sortByLayer)");

    const CommandDrawer draw{.textures = _impl->textures,
                             .fonts = _impl->fonts};
    for (const DrawCommand &command: queue.commands())
        std::visit(draw, command);
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

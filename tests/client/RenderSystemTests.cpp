/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** RenderSystemTests
*/

#include <cstdint>
#include <gtest/gtest.h>
#include <utility>
#include <variant>
#include <vector>

#include "ClientSettings.hpp"
#include "engine/core/Color.hpp"
#include "engine/core/Rect2.hpp"
#include "engine/core/Vec2.hpp"
#include "engine/ecs/Registry.hpp"
#include "engine/render/DrawCommand.hpp"
#include "engine/render/RenderQueue.hpp"
#include "PresentationComponents.hpp"
#include "RenderSystems.hpp"
#include "rtype/game/Components.hpp"

namespace rtype::client {
namespace {
/** Registers the component types the render tests add to entities. */
void registerTypes(engine::ecs::Registry &registry)
{
    registry.registerComponent<rtype::game::Transform>();
    registry.registerComponent<Sprite>();
    registry.registerComponent<HighContrastOutline>();
}

using engine::core::FloatRect;
using engine::core::IntRect;
using engine::core::Vec2f;
using engine::ecs::Entity;
using engine::ecs::Registry;
using engine::render::DrawCommand;
using engine::render::RectCmd;
using engine::render::RenderQueue;
using engine::render::SpriteCmd;
using engine::render::TextureId;

constexpr TextureId SHEET{7};

IntRect frame(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h)
{
    return IntRect({x, y}, w, h);
}

Sprite sprite(IntRect rect, std::int16_t layer)
{
    return Sprite{
        .texture = SHEET,
        .frame   = rect,
        .layer   = layer,
        .tint    = engine::core::Color::WHITE,
        .scale   = Vec2f(1.0F, 1.0F),
    };
}

Entity place(Registry &registry, Vec2f at, Sprite drawn)
{
    const Entity entity = registry.spawnEntity();

    registry.addComponent(entity, rtype::game::Transform{at});
    registry.addComponent(entity, std::move(drawn));
    return entity;
}

/** Runs the render system once into the registry's RenderQueue resource. */
void draw(Registry &registry, bool highContrast)
{
    ClientSettings settings;

    settings.highContrast = highContrast;
    registry.setResource(settings);
    renderSystem(registry, registry.getComponents<rtype::game::Transform>(),
        registry.getComponents<Sprite>());
}

template <typename Command>
std::vector<Command> commandsOf(const RenderQueue &queue)
{
    std::vector<Command> found;

    for (const DrawCommand &command: queue.commands())
        if (const auto *cmd = std::get_if<Command>(&command))
            found.push_back(*cmd);
    return found;
}

const SpriteCmd *findAt(const std::vector<SpriteCmd> &cmds, float x, float y)
{
    for (const SpriteCmd &cmd: cmds)
        if (cmd.position.x() == x && cmd.position.y() == y)
            return &cmd;
    return nullptr;
}

bool covers(const FloatRect &box, float x, float y)
{
    return x >= box.origin().x() && x <= box.origin().x() + box.width() &&
        y >= box.origin().y() && y <= box.origin().y() + box.height();
}
} // namespace

TEST(RenderSystem,
    EachSpriteBecomesOneSpriteCmdWithItsLayerAndFrameCentreOrigin)
{
    Registry registry;
    registerTypes(registry);
    registry.setResource(RenderQueue{});
    place(registry, Vec2f(10.0F, 20.0F), sprite(frame(0, 0, 8, 8), 1));
    place(registry, Vec2f(30.0F, 40.0F), sprite(frame(8, 0, 8, 16), -2));

    draw(registry, false);

    const auto cmds =
        commandsOf<SpriteCmd>(registry.getResource<RenderQueue>());
    ASSERT_EQ(cmds.size(), 2U);

    const SpriteCmd *first  = findAt(cmds, 10.0F, 20.0F);
    const SpriteCmd *second = findAt(cmds, 30.0F, 40.0F);
    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);

    EXPECT_EQ(first->texture, SHEET);
    EXPECT_EQ(first->layer, 1);
    EXPECT_EQ(first->source.origin().x(), 0);
    EXPECT_EQ(first->source.width(), 8);
    // The renderer draws the frame at position - origin, so the origin must
    // be the frame centre to draw the sprite centred on its position.
    EXPECT_FLOAT_EQ(first->origin.x(), 4.0F);
    EXPECT_FLOAT_EQ(first->origin.y(), 4.0F);

    EXPECT_EQ(second->layer, -2);
    EXPECT_EQ(second->source.origin().x(), 8);
    EXPECT_EQ(second->source.height(), 16);
    EXPECT_FLOAT_EQ(second->origin.x(), 4.0F);
    EXPECT_FLOAT_EQ(second->origin.y(), 8.0F);
}

TEST(RenderSystem, AnEntityWithoutASpriteAddsNoCommand)
{
    Registry registry;
    registerTypes(registry);
    registry.setResource(RenderQueue{});
    const Entity bare = registry.spawnEntity();

    registry.addComponent(bare, rtype::game::Transform{Vec2f(5.0F, 5.0F)});
    draw(registry, false);

    EXPECT_TRUE(commandsOf<SpriteCmd>(registry.getResource<RenderQueue>())
            .empty());
}

TEST(RenderSystem, HighContrastOutlinesOnlyTheMarkedEnemy)
{
    Registry registry;
    registerTypes(registry);
    registry.setResource(RenderQueue{});
    const Entity enemy =
        place(registry, Vec2f(600.0F, 300.0F), sprite(frame(0, 0, 20, 24), 0));
    registry.addComponent(enemy, HighContrastOutline{});
    place(registry, Vec2f(100.0F, 100.0F), sprite(frame(0, 0, 33, 17), 0));

    draw(registry, true);

    const auto outlines =
        commandsOf<RectCmd>(registry.getResource<RenderQueue>());
    ASSERT_EQ(outlines.size(), 1U);

    // The enemy is centred on (600, 300) and 20 x 24, so its box is
    // x 590..610 and y 288..312.
    const FloatRect &box = outlines[0].rect;
    EXPECT_LE(box.origin().x(), 590.0F);
    EXPECT_GE(box.origin().x() + box.width(), 610.0F);
    EXPECT_LE(box.origin().y(), 288.0F);
    EXPECT_GE(box.origin().y() + box.height(), 312.0F);
    EXPECT_FALSE(covers(box, 100.0F, 100.0F)) << "the ship must not be boxed";
}

TEST(RenderSystem, NoOutlineIsDrawnWhenHighContrastIsOff)
{
    Registry registry;
    registerTypes(registry);
    registry.setResource(RenderQueue{});
    const Entity enemy =
        place(registry, Vec2f(600.0F, 300.0F), sprite(frame(0, 0, 20, 24), 0));
    registry.addComponent(enemy, HighContrastOutline{});

    draw(registry, false);

    EXPECT_TRUE(commandsOf<RectCmd>(registry.getResource<RenderQueue>())
            .empty());
}

} // namespace rtype::client

/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** RenderSystemTests
*/

#include <gtest/gtest.h>

#include <cstdint>
#include <utility>
#include <variant>
#include <vector>

#include "ClientComponents.hpp"
#include "ClientSettings.hpp"
#include "PresentationComponents.hpp"
#include "RenderSystems.hpp"
#include "engine/core/Color.hpp"
#include "engine/core/Rect2.hpp"
#include "engine/core/Vec2.hpp"
#include "engine/ecs/Registry.hpp"
#include "engine/render/DrawCommand.hpp"
#include "engine/render/RenderQueue.hpp"
#include "rtype/game/Components.hpp"

namespace rtype::client {
namespace {
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
        .frame = rect,
        .layer = layer,
        .tint = engine::core::Color(255, 255, 255, 255),
        .scale = Vec2f(1.0F, 1.0F),
    };
}

Entity place(Registry &registry, Vec2f at, Sprite drawn)
{
    const Entity entity = registry.spawnEntity();

    registry.addComponent(entity, rtype::game::Transform{at});
    registry.addComponent(entity, drawn);
    return entity;
}

void prepare(Registry &registry, bool highContrast)
{
    ClientSettings settings;

    settings.highContrast = highContrast;
    registry.setResource(settings);
    registry.setResource(RenderQueue{});
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
} // namespace

TEST(RenderSystem, EachSpriteBecomesOneSpriteCmdWithItsLayerAndFrameCentreOrigin)
{
    Registry registry;
    prepare(registry, false);
    place(registry, Vec2f(10.0F, 20.0F), sprite(frame(0, 0, 8, 8), 1));
    place(registry, Vec2f(30.0F, 40.0F), sprite(frame(8, 0, 8, 16), -2));

    renderSystem(registry, {}, {});

    const auto cmds = commandsOf<SpriteCmd>(registry.getResource<RenderQueue>());
    ASSERT_EQ(cmds.size(), 2U);

    const SpriteCmd *first = findAt(cmds, 10.0F, 20.0F);
    const SpriteCmd *second = findAt(cmds, 30.0F, 40.0F);
    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);

    EXPECT_EQ(first->texture, SHEET);
    EXPECT_EQ(first->layer, 1);
    EXPECT_EQ(first->source.origin().x(), 0);
    EXPECT_EQ(first->source.width(), 8);
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
    prepare(registry, false);
    const Entity bare = registry.spawnEntity();

    registry.addComponent(bare, rtype::game::Transform{Vec2f(5.0F, 5.0F)});
    renderSystem(registry);

    EXPECT_TRUE(commandsOf<SpriteCmd>(registry.getResource<RenderQueue>())
        .empty());
}

TEST(RenderSystem, HighContrastOutlinesOnlyTheMarkedEnemy)
{
    Registry registry;
    prepare(registry, true);
    const Entity enemy = place(registry, Vec2f(600.0F, 300.0F),
        sprite(frame(0, 0, 20, 24), 0));
    registry.addComponent(enemy, HighContrastOutline{});
    place(registry, Vec2f(100.0F, 100.0F), sprite(frame(0, 0, 33, 17), 0));

    renderSystem(registry);

    const auto outlines = commandsOf<RectCmd>(registry.getResource<RenderQueue>());
    ASSERT_EQ(outlines.size(), 1U);

    const FloatRect &box = outlines[0].rect;
    EXPECT_LE(box.origin().x(), 600.0F);
    EXPECT_GE(box.origin().x() + box.width(), 600.0F);
    EXPECT_LE(box.origin().y(), 300.0F);
    EXPECT_GE(box.origin().y() + box.height(), 300.0F);
    EXPECT_GT(box.origin().x(), 100.0F + 33.0F);  // the ship is not boxed
}

TEST(RenderSystem, NoOutlineIsDrawnWhenHighContrastIsOff)
{
    Registry registry;
    prepare(registry, false);
    const Entity enemy = place(registry, Vec2f(600.0F, 300.0F),
        sprite(frame(0, 0, 20, 24), 0));
    registry.addComponent(enemy, HighContrastOutline{});

    renderSystem(registry);

    EXPECT_TRUE(commandsOf<RectCmd>(registry.getResource<RenderQueue>())
        .empty());
}

} // namespace rtype::client
/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** RenderQueueTests.cpp
*/

#include "RenderQueue.hpp"

#include <cstdint>
#include <gtest/gtest.h>
#include <type_traits>
#include <variant>

#include "DrawCommand.hpp"

TEST(RenderQueueTests, sortByLayerOrdersTheLayers)
{
    const auto getLayers = [](const auto commands) {
        std::vector<std::int16_t> layers;
        layers.reserve(commands.size());

        for (const auto &command: commands) {
            layers
                .push_back(std::visit([](const auto &cmd) { return cmd.layer; },
                    command));
        }
        return layers;
    };

    engine::render::RenderQueue queue;
    std::vector<std::int16_t> expected{0, 10, 120, 1000};

    engine::render::RectCmd a{};
    a.layer = 120;
    queue.push(a);

    engine::render::RectCmd b{};
    b.layer = 0;
    queue.push(b);

    engine::render::RectCmd c{};
    c.layer = 1000;
    queue.push(c);

    engine::render::RectCmd d{};
    d.layer = 10;
    queue.push(d);
    queue.sortByLayer();
    EXPECT_EQ(getLayers(queue.commands()), expected);
}

TEST(RenderQueueTests, preservesRelativeOrderOnEqualLayers)
{
    engine::render::RenderQueue queue;

    engine::render::TextCmd first{
        .font=engine::render::FontId{1}, .text="first", .characterSize=0, .position={}, .color={}, .layer=100,};

    engine::render::RectCmd second{};
    second.layer = 100;
    second.fill  = {1, 2, 3, 4};

    engine::render::SpriteCmd third{
        .texture=engine::render::TextureId{42}, .source={}, .position={}, .origin={}, .scale={}, .rotationDeg={}, .tint={}, .layer=100,};

    engine::render::RectCmd above{};
    above.layer = 1000;
    engine::render::RectCmd below{};
    below.layer = 0;

    queue.push(above);
    queue.push(first);
    queue.push(below);
    queue.push(second);
    queue.push(above);
    queue.push(third);
    queue.sortByLayer();

    const auto commands = queue.commands();
    ASSERT_EQ(commands.size(), 6U);
    EXPECT_EQ(std::get<engine::render::TextCmd>(commands[1]).text, "first");
    EXPECT_EQ(std::get<engine::render::RectCmd>(commands[2]).fill,
        (engine::core::Color{1, 2, 3, 4}));
    EXPECT_EQ(std::get<engine::render::SpriteCmd>(commands[3]).texture.value,
        42U);
}

TEST(RenderQueueTests, preservesCommandTypesAndFields)
{
    engine::render::RenderQueue queue;

    engine::render::SpriteCmd sprite{.texture = engine::render::TextureId{7},
        .source                               = {},
        .position                             = {},
        .origin                               = {},
        .scale                                = {},
        .rotationDeg                          = {},
        .tint                                 = {},
        .layer                                = 10};

    engine::render::RectCmd rect{};
    rect.layer            = 20;
    rect.outlineThickness = 3.5F;

    engine::render::TextCmd text{.font = engine::render::FontId{9},
        .text                          = "hello",
        .characterSize                 = 0,
        .position                      = {},
        .color                         = {},
        .layer                         = 30};

    queue.push(text);
    queue.push(sprite);
    queue.push(rect);
    queue.sortByLayer();

    const auto commands = queue.commands();
    ASSERT_EQ(commands.size(), 3U);
    EXPECT_EQ(std::get<engine::render::SpriteCmd>(commands[0]).texture.value,
        7U);
    EXPECT_FLOAT_EQ(std::get<engine::render::RectCmd>(commands[1])
                        .outlineThickness,
        3.5F);
    EXPECT_EQ(std::get<engine::render::TextCmd>(commands[2]).text, "hello");
    EXPECT_EQ(std::get<engine::render::TextCmd>(commands[2]).font.value, 9U);
}

TEST(RenderQueueTests, clearKeepsCapacity)
{
    engine::render::RenderQueue queue;
    queue.push(engine::render::RectCmd{});
    const auto capacity = queue.capacity();

    queue.clear();

    EXPECT_TRUE(queue.commands().empty());
    EXPECT_EQ(queue.capacity(), capacity);
}

static_assert(!std::is_convertible_v<engine::render::TextureId,
    engine::render::FontId>);
static_assert(!std::is_convertible_v<engine::render::FontId,
    engine::render::TextureId>);
static_assert(engine::render::FontId{}.isDefault());

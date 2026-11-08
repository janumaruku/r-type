/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** InterpolationTests
*/

#include <gtest/gtest.h>

#include <initializer_list>

#include "ClientComponents.hpp"
#include "ClientStages.hpp"
#include "InterpolationSystem.hpp"
#include "engine/ecs/Registry.hpp"
#include "rtype/game/Components.hpp"

using engine::core::Vec2f;
using rtype::client::LocalPlayer;
using rtype::client::NetSample;
using rtype::client::NetSamples;
using rtype::client::RenderClock;
using rtype::game::Transform;

/*
** A registry with the components interpolationSystem reads, and a
** RenderClock set so that renderTick() is latest - 6.
*/
class Interpolation: public testing::Test {
protected:
    void SetUp() override
    {
        _registry.registerComponent<Transform>();
        _registry.registerComponent<NetSamples>();
        _registry.registerComponent<LocalPlayer>();
        _registry.emplaceResource<RenderClock>();
    }

    engine::ecs::Entity spawn(std::initializer_list<NetSample> samples)
    {
        const auto entity = _registry.spawnEntity();
        NetSamples net;
        std::size_t index = 0;

        for (const auto &sample: samples)
            net.ring.at(index++) = sample;
        net.count = static_cast<std::uint8_t>(index);
        _registry.addComponent(entity, Transform{{-1.0F, -1.0F}});
        _registry.addComponent(entity, std::move(net));
        return entity;
    }

    void runAt(std::uint32_t renderTick)
    {
        _registry.getResource<RenderClock>().latestServerTick =
            renderTick + rtype::client::INTERPOLATION_DELAY_TICKS;
        rtype::client::interpolationSystem(_registry,
            _registry.getComponents<Transform>(),
            _registry.getComponents<NetSamples>());
    }

    Vec2f positionOf(const engine::ecs::Entity &entity)
    {
        return _registry.getComponent<Transform>(entity)->position;
    }

    engine::ecs::Registry _registry;
};

TEST(RenderClock, LatestTick100And8MsSinceGives94Point48)
{
    const RenderClock clock{.latestServerTick = 100, .sinceLatestTick = 0.008F};

    EXPECT_NEAR(clock.renderTick(), 94.48F, 1e-4F);
}

TEST(RenderClock, NeverBelowZero)
{
    const RenderClock clock{.latestServerTick = 2, .sinceLatestTick = 0.0F};

    EXPECT_FLOAT_EQ(clock.renderTick(), 0.0F);
}

TEST_F(Interpolation, BetweenTwoSamplesGivesTheMidpoint)
{
    const auto entity = spawn({
        {.tick = 100, .position = {0.0F, 10.0F}, .velocity = {}},
        {.tick = 102, .position = {20.0F, 30.0F}, .velocity = {}},
    });

    runAt(101);
    EXPECT_FLOAT_EQ(positionOf(entity).x(), 10.0F);
    EXPECT_FLOAT_EQ(positionOf(entity).y(), 20.0F);
}

TEST_F(Interpolation, AfterTheNewestSampleExtrapolatesAtMostSixTicks)
{
    const auto entity = spawn({
        {.tick = 98, .position = {98.0F, 50.0F}, .velocity = {60.0F, 0.0F}},
        {.tick = 100, .position = {100.0F, 50.0F}, .velocity = {60.0F, 0.0F}},
    });

    runAt(103);
    EXPECT_FLOAT_EQ(positionOf(entity).x(), 103.0F);
    runAt(110);
    EXPECT_FLOAT_EQ(positionOf(entity).x(), 106.0F);
    EXPECT_FLOAT_EQ(positionOf(entity).y(), 50.0F);
}

TEST_F(Interpolation, OneSampleOnlyIsDrawnAtThatSample)
{
    const auto entity = spawn({
        {.tick = 100, .position = {40.0F, 60.0F}, .velocity = {60.0F, 0.0F}},
    });

    runAt(103);
    EXPECT_FLOAT_EQ(positionOf(entity).x(), 40.0F);
    EXPECT_FLOAT_EQ(positionOf(entity).y(), 60.0F);
}

TEST_F(Interpolation, BeforeTheOldestSampleIsDrawnAtTheOldest)
{
    const auto entity = spawn({
        {.tick = 100, .position = {40.0F, 60.0F}, .velocity = {}},
        {.tick = 102, .position = {80.0F, 60.0F}, .velocity = {}},
    });

    runAt(95);
    EXPECT_FLOAT_EQ(positionOf(entity).x(), 40.0F);
}

TEST_F(Interpolation, TheLocalShipIsNotMoved)
{
    const auto entity = spawn({
        {.tick = 100, .position = {0.0F, 0.0F}, .velocity = {}},
        {.tick = 102, .position = {20.0F, 0.0F}, .velocity = {}},
    });

    _registry.addComponent(entity, LocalPlayer{});
    runAt(101);
    EXPECT_FLOAT_EQ(positionOf(entity).x(), -1.0F);
    EXPECT_FLOAT_EQ(positionOf(entity).y(), -1.0F);
}

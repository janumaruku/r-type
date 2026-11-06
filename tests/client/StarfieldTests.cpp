/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** StarfieldTests
*/

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <vector>

#include "RenderSystems.hpp"
#include "Starfield.hpp"
#include "engine/core/Rng.hpp"
#include "engine/core/Vec2.hpp"
#include "engine/ecs/Registry.hpp"
#include "engine/ecs/Resources.hpp"
#include "engine/render/DrawCommand.hpp"
#include "rtype/game/Components.hpp"

namespace rtype::client {
namespace {
using engine::core::Rng;
using engine::core::Vec2f;
using engine::ecs::DeltaTime;
using engine::ecs::Entity;
using engine::ecs::Registry;
using engine::render::TextureId;
using rtype::game::Transform;

constexpr float PLAYFIELD_WIDTH = 1280.0F;
constexpr float PLAYFIELD_HEIGHT = 720.0F;
constexpr TextureId WHITE_PIXEL{0};

/** Registers the component types the starfield tests add to entities. */
void registerTypes(Registry &registry)
{
    registry.registerComponent<Transform>();
    registry.registerComponent<Star>();
    registry.registerComponent<Sprite>();
}

struct Layers {
    int farLayer = 0;
    int midLayer = 0;
    int nearLayer = 0;
    int onScreen = 0;
};

Layers countLayers(Registry &registry)
{
    Layers layers;

    for (auto &&[star, transform]: registry.view<Star, Transform>()) {
        if (star.speed == STAR_LAYERS[0].speed)
            ++layers.farLayer;
        else if (star.speed == STAR_LAYERS[1].speed)
            ++layers.midLayer;
        else if (star.speed == STAR_LAYERS[2].speed)
            ++layers.nearLayer;

        if (transform.position.x() >= 0.0F
            && transform.position.x() <= PLAYFIELD_WIDTH
            && transform.position.y() >= 0.0F
            && transform.position.y() <= PLAYFIELD_HEIGHT)
            ++layers.onScreen;
    }
    return layers;
}

std::vector<float> xPositions(Registry &registry)
{
    std::vector<float> xs;

    for (auto &&[star, transform]: registry.view<Star, Transform>()) {
        (void)star;
        xs.push_back(transform.position.x());
    }
    return xs;
}

void scrollFor(Registry &registry, Rng &rng, int frames, float dt)
{
    for (int frame = 0; frame < frames; ++frame) {
        registry.setResource(DeltaTime{dt});
        starfieldSystem(registry, registry.getComponents<Transform>(),
            registry.getComponents<Star>(), rng);
    }
}
} // namespace

TEST(Starfield, SpawnsOneHundredThirtyStarsInThreeSpeedLayers)
{
    Registry registry;
    registerTypes(registry);
    Rng rng{1U};

    spawnStarfield(registry, rng, WHITE_PIXEL);

    const Layers layers = countLayers(registry);
    EXPECT_EQ(registry.aliveCount(), 130U);
    EXPECT_EQ(layers.farLayer, 70);
    EXPECT_EQ(layers.midLayer, 40);
    EXPECT_EQ(layers.nearLayer, 20);
    EXPECT_EQ(layers.onScreen, 130);
}

TEST(Starfield, ScrollingCoversTheSameDistanceAt30AndAt144FramesPerSecond)
{
    Registry slow;
    registerTypes(slow);
    Registry fast;
    registerTypes(fast);
    Rng slowRng{42U};
    Rng fastRng{42U};
    spawnStarfield(slow, slowRng, WHITE_PIXEL);
    spawnStarfield(fast, fastRng, WHITE_PIXEL);
    ASSERT_EQ(xPositions(slow), xPositions(fast));

    scrollFor(slow, slowRng, 30, 1.0F / 30.0F);     // one second at 30 FPS
    scrollFor(fast, fastRng, 144, 1.0F / 144.0F);   // one second at 144 FPS

    const std::vector<float> slowXs = xPositions(slow);
    const std::vector<float> fastXs = xPositions(fast);
    ASSERT_EQ(slowXs.size(), 130U);
    ASSERT_EQ(fastXs.size(), slowXs.size());
    for (std::size_t star = 0; star < slowXs.size(); ++star)
        EXPECT_NEAR(slowXs[star], fastXs[star], 0.01F) << "star " << star;
}

TEST(Starfield, AStarMovesLeftBySpeedTimesTheElapsedTime)
{
    Registry registry;
    registerTypes(registry);
    Rng rng{1U};
    const Entity star = registry.spawnEntity();

    registry.addComponent(star, Transform{Vec2f(500.0F, 40.0F)});
    registry.addComponent(star, Star{STAR_LAYERS[1].speed});  // 55 u/s
    registry.setResource(DeltaTime{1.0F});

    starfieldSystem(registry, registry.getComponents<Transform>(),
        registry.getComponents<Star>(), rng);

    const Vec2f position = registry.getComponent<Transform>(star)->position;
    EXPECT_NEAR(position.x(), 500.0F - STAR_LAYERS[1].speed, 1e-3F);
    EXPECT_NEAR(position.y(), 40.0F, 1e-3F);
}

TEST(Starfield, AStarCrossingTheLeftEdgeReappearsNearTheRightEdge)
{
    Registry registry;
    registerTypes(registry);
    Rng rng{1U};
    const Entity star = registry.spawnEntity();

    // 2 - 120 * 0.05 = -4: the star has crossed x = 0.
    registry.addComponent(star, Transform{Vec2f(2.0F, 100.0F)});
    registry.addComponent(star, Star{STAR_LAYERS[2].speed});  // 120 u/s
    registry.setResource(DeltaTime{0.05F});

    starfieldSystem(registry, registry.getComponents<Transform>(),
        registry.getComponents<Star>(), rng);

    const Vec2f position = registry.getComponent<Transform>(star)->position;
    EXPECT_NEAR(position.x(), PLAYFIELD_WIDTH, 10.0F);
    EXPECT_GE(position.y(), 0.0F);  // a new random y on the wrap
    EXPECT_LE(position.y(), PLAYFIELD_HEIGHT);
}

} // namespace rtype::client
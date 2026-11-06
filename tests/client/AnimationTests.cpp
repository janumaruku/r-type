/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** AnimationTests
*/

#include <gtest/gtest.h>

#include <nlohmann/json.hpp>
#include <raylib.h>

#include <cmath>
#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <utility>

#include "ClientComponents.hpp"
#include "PresentationComponents.hpp"
#include "SpriteCatalog.hpp"
#include "engine/core/Rect2.hpp"
#include "engine/core/Vec2.hpp"
#include "engine/ecs/Registry.hpp"
#include "engine/ecs/Resources.hpp"
#include "engine/render/TextureCache.hpp"
#include "rtype/game/Components.hpp"

namespace rtype::client {
namespace {
using engine::core::IntRect;
using engine::core::Vec2f;
using engine::ecs::DeltaTime;
using engine::ecs::Entity;
using engine::ecs::Registry;
using engine::render::TextureCache;

/*
** Ship states in index order: idle 0 (3 frames), up 1 (1 frame), top 2
** (3 frames), down 3 (4 frames), bottom 4 (1 frame). Frames are 8 x 8.
*/
constexpr std::string_view FULL_SHIP = R"({
    "idle": [[0, 0, 8, 8], [8, 0, 8, 8], [16, 0, 8, 8]],
    "up": [[0, 8, 8, 8]],
    "top": [[8, 8, 8, 8], [16, 8, 8, 8], [24, 8, 8, 8]],
    "down": [[0, 16, 8, 8], [8, 16, 8, 8], [16, 16, 8, 8], [24, 16, 8, 8]],
    "bottom": [[0, 24, 8, 8]]
})";

constexpr std::string_view IDLE_ONLY_SHIP = R"({
    "idle": [[0, 0, 8, 8], [8, 0, 8, 8], [16, 0, 8, 8]]
})";

/*
** A catalog with every key the game needs. The explosion does not loop.
*/
std::string catalogText(std::string_view shipStates)
{
    return std::string(R"({"sprites": [
        {"kind": "PLAYER_SHIP", "variant": "*", "sheet": "sheet.png",
         "frameTime": 0.1, "states": )")
        + std::string(shipStates) + R"(},
        {"kind": "BYDOS_SLAVE", "variant": "*", "sheet": "sheet.png",
         "frameTime": 0.1, "states": {"idle": [[0, 0, 8, 8]]}},
        {"kind": "PLAYER_MISSILE", "variant": "*", "sheet": "sheet.png",
         "frameTime": 0.0, "states": {"idle": [[0, 0, 8, 8]]}},
        {"kind": "ENEMY_MISSILE", "variant": "*", "sheet": "sheet.png",
         "frameTime": 0.1, "states": {"idle": [[0, 0, 8, 8]]}},
        {"kind": "EXPLOSION", "variant": "*", "sheet": "sheet.png",
         "frameTime": 0.1, "loop": false,
         "states": {"idle": [[0, 0, 8, 8], [8, 0, 8, 8], [16, 0, 8, 8]]}}
    ]})");
}

class AnimationFixture: public ::testing::Test {
protected:
    void SetUp() override
    {
        SetConfigFlags(FLAG_WINDOW_HIDDEN);
        SetTraceLogLevel(LOG_NONE);
        InitWindow(64, 64, "r-type animation tests");
        if (!IsWindowReady())
            GTEST_SKIP() << "no display: loading a sprite sheet needs a window";

        _dir = std::filesystem::temp_directory_path() / "rtype-animation-tests";
        std::filesystem::create_directories(_dir);
        Image sheet = GenImageColor(64, 64, BLANK);
        ExportImage(sheet, (_dir / "sheet.png").string().c_str());
        UnloadImage(sheet);
    }

    void TearDown() override
    {
        if (IsWindowReady())
            CloseWindow();
    }

    /** Loads the test catalog into `registry` as its SpriteCatalog resource. */
    const SpriteCatalog &loadCatalog(Registry &registry, TextureCache &textures,
        std::string_view shipStates) const
    {
        SpriteCatalog catalog;

        catalog.loadJson(nlohmann::ordered_json::parse(catalogText(shipStates)),
            _dir, textures);
        registry.setResource(std::move(catalog));
        return registry.getResource<SpriteCatalog>();
    }

    std::filesystem::path _dir;
};

Entity spawnAnimated(Registry &registry, const SpriteCatalog &catalog,
    SpriteKey key)
{
    const Entity entity = registry.spawnEntity();
    const SpriteDefId def = catalog.find(key, 0);
    const SpriteDef &spec = catalog.get(def);

    registry.addComponent(entity, Animation{.def = def});
    registry.addComponent(entity, Sprite{
        .texture = spec.texture,
        .frame = spec.states.front().frames.front(),
        .layer = 0,
        .tint = engine::core::Color(255, 255, 255, 255),
        .scale = Vec2f(1.0F, 1.0F),
    });
    return entity;
}

Entity spawnShip(Registry &registry, const SpriteCatalog &catalog, float y)
{
    const Entity ship = spawnAnimated(registry, catalog, SpriteKey::PLAYER_SHIP);

    registry.addComponent(ship, rtype::game::Transform{Vec2f(100.0F, y)});
    registry.addComponent(ship, ShipTilt{.lastY = y});
    return ship;
}

void tick(Registry &registry, float seconds)
{
    registry.setResource(DeltaTime{seconds});
    animationSystem(registry);
}

/** Moves the ship vertically at `speed` u/s for one step, then runs the tilt. */
void moveShip(Registry &registry, Entity ship, float speed, float dt)
{
    auto &position = registry.getComponent<rtype::game::Transform>(ship)->position;

    position = Vec2f(position.x(), position.y() + speed * dt);
    registry.setResource(DeltaTime{dt});
    shipTiltSystem(registry);
}

std::string stateName(const Registry &registry, Entity ship)
{
    const auto &animation = *registry.getComponent<Animation>(ship);
    const auto &catalog = registry.getResource<SpriteCatalog>();

    return catalog.get(animation.def).states[animation.state].name;
}

void expectRect(const IntRect &rect, std::int32_t x, std::int32_t y,
    std::int32_t w, std::int32_t h)
{
    EXPECT_EQ(rect.origin().x(), x);
    EXPECT_EQ(rect.origin().y(), y);
    EXPECT_EQ(rect.width(), w);
    EXPECT_EQ(rect.height(), h);
}
} // namespace

/* ---- Animation ---------------------------------------------------------- */

TEST_F(AnimationFixture, ThreeFramesAt0Point1SecondsReachFrameTwoInOneLongStep)
{
    TextureCache textures;
    Registry registry;
    const SpriteCatalog &sprites = loadCatalog(registry, textures, FULL_SHIP);
    const Entity ship = spawnAnimated(registry, sprites, SpriteKey::PLAYER_SHIP);

    tick(registry, 0.25F);

    EXPECT_EQ(registry.getComponent<Animation>(ship)->index, 2U);
}

TEST_F(AnimationFixture, TwentyFiveSmallStepsReachTheSameFrame)
{
    TextureCache textures;
    Registry registry;
    const SpriteCatalog &sprites = loadCatalog(registry, textures, FULL_SHIP);
    const Entity ship = spawnAnimated(registry, sprites, SpriteKey::PLAYER_SHIP);

    for (int step = 0; step < 25; ++step)
        tick(registry, 0.01F);

    EXPECT_EQ(registry.getComponent<Animation>(ship)->index, 2U);
}

TEST_F(AnimationFixture, ANonLoopingStateStopsOnItsLastFrame)
{
    TextureCache textures;
    Registry registry;
    const SpriteCatalog &sprites = loadCatalog(registry, textures, FULL_SHIP);
    const Entity explosion = spawnAnimated(registry, sprites,
        SpriteKey::EXPLOSION);

    tick(registry, 1.0F);

    EXPECT_EQ(registry.getComponent<Animation>(explosion)->index, 2U);
}

TEST_F(AnimationFixture, TheSpriteShowsTheFrameOfTheCurrentIndex)
{
    TextureCache textures;
    Registry registry;
    const SpriteCatalog &sprites = loadCatalog(registry, textures, FULL_SHIP);
    const Entity ship = spawnAnimated(registry, sprites, SpriteKey::PLAYER_SHIP);

    tick(registry, 0.15F);

    EXPECT_EQ(registry.getComponent<Animation>(ship)->index, 1U);
    expectRect(registry.getComponent<Sprite>(ship)->frame, 8, 0, 8, 8);
}

TEST_F(AnimationFixture, SettingTheCurrentStateKeepsItsFrameAndTimer)
{
    TextureCache textures;
    Registry registry;
    const SpriteCatalog &sprites = loadCatalog(registry, textures, FULL_SHIP);
    const Entity ship = spawnAnimated(registry, sprites, SpriteKey::PLAYER_SHIP);
    auto &animation = *registry.getComponent<Animation>(ship);

    ASSERT_TRUE(setAnimationState(animation, sprites, "top"));  // state 2
    animation.index = 2;
    animation.timer = 0.03F;

    EXPECT_TRUE(setAnimationState(animation, sprites, "top"));
    EXPECT_EQ(animation.state, 2);
    EXPECT_EQ(animation.index, 2U);
    EXPECT_FLOAT_EQ(animation.timer, 0.03F);
}

TEST_F(AnimationFixture, AnUnknownStateNameIsRefusedAndChangesNothing)
{
    TextureCache textures;
    Registry registry;
    const SpriteCatalog &sprites = loadCatalog(registry, textures, FULL_SHIP);
    const Entity ship = spawnAnimated(registry, sprites, SpriteKey::PLAYER_SHIP);
    auto &animation = *registry.getComponent<Animation>(ship);

    animation.index = 1;
    animation.timer = 0.02F;

    EXPECT_FALSE(setAnimationState(animation, sprites, "spin"));
    EXPECT_EQ(animation.state, 0);
    EXPECT_EQ(animation.index, 1U);
    EXPECT_FLOAT_EQ(animation.timer, 0.02F);
}

TEST_F(AnimationFixture, ANewStateStartsAtItsFirstFrameAndIsDrawnThatWay)
{
    TextureCache textures;
    Registry registry;
    const SpriteCatalog &sprites = loadCatalog(registry, textures, FULL_SHIP);
    const Entity ship = spawnAnimated(registry, sprites, SpriteKey::PLAYER_SHIP);
    auto &animation = *registry.getComponent<Animation>(ship);

    ASSERT_TRUE(setAnimationState(animation, sprites, "down"));  // 4 frames, state 3
    animation.index = 2;
    animation.timer = 0.05F;

    ASSERT_TRUE(setAnimationState(animation, sprites, "up"));  // 1 frame, state 1
    tick(registry, 0.01F);

    EXPECT_EQ(animation.state, 1);
    EXPECT_EQ(animation.index, 0U);
    expectRect(registry.getComponent<Sprite>(ship)->frame, 0, 8, 8, 8);
}

/* ---- Ship tilt ---------------------------------------------------------- */

TEST_F(AnimationFixture, AShortBurstUpShowsUpAndThenIdleWhenItStops)
{
    /*
    ** 0.1 s is under the 0.15 s that the full tilt needs, so the ship only
    ** shows `up`, then goes back to idle once it stops.
    */
    TextureCache textures;
    Registry registry;
    const SpriteCatalog &sprites = loadCatalog(registry, textures, FULL_SHIP);
    const Entity ship = spawnShip(registry, sprites, 300.0F);

    moveShip(registry, ship, -300.0F, 0.05F);
    moveShip(registry, ship, -300.0F, 0.05F);
    EXPECT_EQ(stateName(registry, ship), "up");

    moveShip(registry, ship, 0.0F, 0.05F);
    EXPECT_EQ(stateName(registry, ship), "idle");
}

TEST_F(AnimationFixture, HoldingUpPastFifteenHundredthsOfASecondShowsTheFullTilt)
{
    TextureCache textures;
    Registry registry;
    const SpriteCatalog &sprites = loadCatalog(registry, textures, FULL_SHIP);
    const Entity ship = spawnShip(registry, sprites, 300.0F);

    moveShip(registry, ship, -300.0F, 0.05F);
    EXPECT_EQ(stateName(registry, ship), "up");

    for (int step = 0; step < 3; ++step)
        moveShip(registry, ship, -300.0F, 0.05F);  // 0.2 s held in total
    EXPECT_EQ(stateName(registry, ship), "top");

    moveShip(registry, ship, 0.0F, 0.05F);
    EXPECT_EQ(stateName(registry, ship), "idle");
}

TEST_F(AnimationFixture, MovingDownAfterUpRestartsTheHeldTime)
{
    TextureCache textures;
    Registry registry;
    const SpriteCatalog &sprites = loadCatalog(registry, textures, FULL_SHIP);
    const Entity ship = spawnShip(registry, sprites, 300.0F);

    moveShip(registry, ship, -300.0F, 0.05F);
    moveShip(registry, ship, -300.0F, 0.05F);
    moveShip(registry, ship, 300.0F, 0.05F);

    EXPECT_EQ(stateName(registry, ship), "down");
    EXPECT_NEAR(registry.getComponent<ShipTilt>(ship)->heldTime, 0.05F, 1e-4F);
}

TEST_F(AnimationFixture, VerticalSpeedUnderTwentyUnitsPerSecondStaysIdle)
{
    TextureCache textures;
    Registry registry;
    const SpriteCatalog &sprites = loadCatalog(registry, textures, FULL_SHIP);
    const Entity ship = spawnShip(registry, sprites, 300.0F);

    for (int step = 0; step < 10; ++step) {
        moveShip(registry, ship, 10.0F, 0.05F);
        EXPECT_EQ(stateName(registry, ship), "idle") << "step " << step;
    }
}

TEST_F(AnimationFixture, AShipWhoseSpriteHasOnlyIdleStaysIdleWhenMovingUp)
{
    TextureCache textures;
    Registry registry;
    const SpriteCatalog &sprites = loadCatalog(registry, textures,
        IDLE_ONLY_SHIP);
    const Entity ship = spawnShip(registry, sprites, 300.0F);

    EXPECT_NO_THROW(moveShip(registry, ship, -300.0F, 0.05F));
    EXPECT_NO_THROW(moveShip(registry, ship, -300.0F, 0.05F));
    EXPECT_EQ(stateName(registry, ship), "idle");
}

TEST_F(AnimationFixture, ZeroDeltaTimeChangesNothingAndLeavesNoNaN)
{
    TextureCache textures;
    Registry registry;
    const SpriteCatalog &sprites = loadCatalog(registry, textures, FULL_SHIP);
    const Entity ship = spawnShip(registry, sprites, 200.0F);

    // A 50 unit jump in no time: the speed would be infinite.
    registry.getComponent<rtype::game::Transform>(ship)->position =
        Vec2f(100.0F, 150.0F);
    registry.setResource(DeltaTime{0.0F});

    EXPECT_NO_THROW(shipTiltSystem(registry));
    EXPECT_EQ(stateName(registry, ship), "idle");
    EXPECT_TRUE(std::isfinite(registry.getComponent<ShipTilt>(ship)->heldTime));
}

} // namespace rtype::client
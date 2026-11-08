/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** EffectsReactorTests
*/

#include "EffectsReactor.hpp"

#include <cstdint>
#include <filesystem>
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include <string>
#include <string_view>
#include <utility>

#include "ClientSettings.hpp"
#include "DisplayHelpers.hpp"
#include "engine/core/Color.hpp"
#include "engine/core/Vec2.hpp"
#include "engine/ecs/Registry.hpp"
#include "engine/ecs/Resources.hpp"
#include "engine/render/TextureCache.hpp"
#include "GameEventBus.hpp"
#include "PresentationComponents.hpp"
#include "RenderSystems.hpp"
#include "rtype/game/Components.hpp"
#include "rtype/protocol/Event.hpp"
#include "SpriteCatalog.hpp"
#include "Starfield.hpp"

// raylib defines WHITE, BLANK and friends as macros, so it must come after
// every engine header.
#include <raylib.h>

namespace rtype::client {
namespace {
using engine::core::Vec2f;
using engine::ecs::DeltaTime;
using engine::ecs::Entity;
using engine::ecs::Registry;
using engine::render::TextureCache;
using protocol::Event;
using protocol::EventType;

constexpr float EXPLOSION_FRAME_TIME   = 0.05F;
constexpr std::size_t EXPLOSION_FRAMES = 6;

/*
** Every kind the game needs. The explosion has 6 frames of 0.05 s and does
** not loop, so it lasts 0.3 s.
*/
constexpr std::string_view CATALOG = R"({"sprites": [
    {"kind": "PLAYER_SHIP", "variant": "*", "sheet": "sheet.png",
     "frameTime": 0.1, "states": {"idle": [[0, 0, 8, 8]]}},
    {"kind": "BYDOS_SLAVE", "variant": "*", "sheet": "sheet.png",
     "frameTime": 0.1, "states": {"idle": [[0, 0, 8, 8]]}},
    {"kind": "PLAYER_MISSILE", "variant": "*", "sheet": "sheet.png",
     "frameTime": 0.0, "states": {"idle": [[0, 0, 8, 8]]}},
    {"kind": "ENEMY_MISSILE", "variant": "*", "sheet": "sheet.png",
     "frameTime": 0.1, "states": {"idle": [[0, 0, 8, 8]]}},
    {"kind": "EXPLOSION", "variant": "*", "sheet": "sheet.png",
     "frameTime": 0.05, "loop": false,
     "states": {"idle": [[0, 0, 8, 8], [8, 0, 8, 8], [16, 0, 8, 8],
                         [24, 0, 8, 8], [32, 0, 8, 8], [40, 0, 8, 8]]}}
]})";

void registerTypes(Registry &registry)
{
    registry.registerComponent<rtype::game::Transform>();
    registry.registerComponent<Animation>();
    registry.registerComponent<Sprite>();
    registry.registerComponent<TransientEffect>();
}

Event event(EventType type, float x = 0.0F, float y = 0.0F)
{
    Event value;

    value.type = type;
    value.x    = x;
    value.y    = y;
    return value;
}

void emit(GameEventBus &bus, const Event &value)
{
    bus.publish(value);
    bus.dispatch();
}

/** Runs one frame of effectSystem, then applies the kills it recorded. */
void tick(Registry &registry, float dt)
{
    registry.setResource(DeltaTime{dt});
    effectSystem(registry, registry.getComponents<TransientEffect>());
    registry.applyCommands();
}

class EffectsFixture: public ::testing::Test {
protected:
    void SetUp() override
    {
        if (!client_tests::hasDisplay())
            GTEST_SKIP() << "no display: loading a sprite sheet needs a window";
        SetConfigFlags(FLAG_WINDOW_HIDDEN);
        SetTraceLogLevel(LOG_NONE);
        InitWindow(64, 64, "r-type effects tests");
        if (!IsWindowReady())
            GTEST_SKIP() << "no display: loading a sprite sheet needs a window";

        _dir = std::filesystem::temp_directory_path() / "rtype-effects-tests";
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

    /** A registry holding the sprite catalog, as the client builds it. */
    void setUpWorld(Registry &registry, TextureCache &textures) const
    {
        SpriteCatalog catalog;

        registerTypes(registry);
        catalog.loadJson(nlohmann::ordered_json::parse(CATALOG), _dir,
            textures);
        registry.setResource(std::move(catalog));
    }

    std::filesystem::path _dir;
};

/** The explosion entities: their position, their animation and their sprite. */
struct Explosion {
    Entity entity;
    Vec2f position;
    engine::core::Color tint;
    float scale;
    std::int16_t layer;
};

std::vector<Explosion> explosions(Registry &registry)
{
    std::vector<Explosion> found;

    for (auto &&[entity, transform, sprite, effect]: registry
             .indexedView<rtype::game::Transform, Sprite, TransientEffect>()) {
        (void)effect;
        found.push_back({
            .entity   = entity,
            .position = transform.position,
            .tint     = sprite.tint,
            .scale    = sprite.scale.x(),
            .layer    = sprite.layer,
        });
    }
    return found;
}
} // namespace

TEST_F(EffectsFixture, EnemyKilledSpawnsOneSmallExplosionAtItsPosition)
{
    TextureCache textures;
    Registry registry;
    ClientSettings settings;
    GameEventBus bus;
    setUpWorld(registry, textures);
    EffectsReactor reactor{bus, registry, settings};

    emit(bus, event(EventType::ENEMY_KILLED, 400.0F, 300.0F));

    const auto found = explosions(registry);
    ASSERT_EQ(found.size(), 1U);
    EXPECT_FLOAT_EQ(found[0].position.x(), 400.0F);
    EXPECT_FLOAT_EQ(found[0].position.y(), 300.0F);
    EXPECT_FLOAT_EQ(found[0].scale, EffectsReactor::SMALL_EXPLOSION_SCALE);
    EXPECT_EQ(found[0].layer, layers::EXPLOSIONS);
    EXPECT_EQ(registry.getComponent<Animation>(found[0].entity)->state, 0);
    EXPECT_NEAR(registry.getComponent<TransientEffect>(found[0].entity)
                    ->remaining,
        static_cast<float>(EXPLOSION_FRAMES) * EXPLOSION_FRAME_TIME, 1e-4F);
}

TEST_F(EffectsFixture, PlayerKilledSpawnsABigExplosion)
{
    TextureCache textures;
    Registry registry;
    ClientSettings settings;
    GameEventBus bus;
    setUpWorld(registry, textures);
    EffectsReactor reactor{bus, registry, settings};

    emit(bus, event(EventType::PLAYER_KILLED, 100.0F, 50.0F));

    const auto found = explosions(registry);
    ASSERT_EQ(found.size(), 1U);
    EXPECT_FLOAT_EQ(found[0].scale, EffectsReactor::BIG_EXPLOSION_SCALE);
    EXPECT_FLOAT_EQ(found[0].scale, 2.0F);
}

TEST_F(EffectsFixture, TheExplosionIsGoneOnceItsAnimationHasEnded)
{
    TextureCache textures;
    Registry registry;
    ClientSettings settings;
    GameEventBus bus;
    setUpWorld(registry, textures);
    EffectsReactor reactor{bus, registry, settings};
    emit(bus, event(EventType::ENEMY_KILLED, 400.0F, 300.0F));
    const Entity explosion = explosions(registry).at(0).entity;

    // Five 0.05 s frames (0.25 s): still playing. Two more (0.35 s): gone.
    for (int frame = 0; frame < 5; ++frame)
        tick(registry, EXPLOSION_FRAME_TIME);
    EXPECT_TRUE(registry.isAlive(explosion));

    for (int frame = 0; frame < 2; ++frame)
        tick(registry, EXPLOSION_FRAME_TIME);
    EXPECT_FALSE(registry.isAlive(explosion));
}

TEST_F(EffectsFixture, ReduceFlashesHalvesTheExplosionAlpha)
{
    TextureCache textures;
    Registry registry;
    ClientSettings reduced;
    GameEventBus bus;
    setUpWorld(registry, textures);
    reduced.reduceFlashes = true;
    EffectsReactor reactor{bus, registry, reduced};

    emit(bus, event(EventType::ENEMY_KILLED, 400.0F, 300.0F));

    const auto found = explosions(registry);
    ASSERT_EQ(found.size(), 1U);
    EXPECT_EQ(found[0].tint.a, 255 / 2);
}

TEST_F(EffectsFixture, AnExplosionKeepsFullAlphaWithoutReduceFlashes)
{
    TextureCache textures;
    Registry registry;
    ClientSettings settings;
    GameEventBus bus;
    setUpWorld(registry, textures);
    EffectsReactor reactor{bus, registry, settings};

    emit(bus, event(EventType::ENEMY_KILLED, 400.0F, 300.0F));

    const auto found = explosions(registry);
    ASSERT_EQ(found.size(), 1U);
    EXPECT_EQ(found[0].tint.a, 255);
}

TEST_F(EffectsFixture, EventsWithoutAnExplosionSpawnNothing)
{
    TextureCache textures;
    Registry registry;
    ClientSettings settings;
    GameEventBus bus;
    setUpWorld(registry, textures);
    EffectsReactor reactor{bus, registry, settings};

    emit(bus, event(EventType::PLAYER_FIRED));
    emit(bus, event(EventType::ENEMY_FIRED));
    emit(bus, event(EventType::GAME_OVER));

    EXPECT_EQ(registry.aliveCount(), 0U);
}

TEST_F(EffectsFixture, ADestroyedReactorSpawnsNothingAndDoesNotCrash)
{
    TextureCache textures;
    Registry registry;
    ClientSettings settings;
    GameEventBus bus;
    setUpWorld(registry, textures);
    {
        EffectsReactor reactor{bus, registry, settings};
    }

    emit(bus, event(EventType::ENEMY_KILLED, 400.0F, 300.0F));

    EXPECT_EQ(registry.aliveCount(), 0U);
    EXPECT_EQ(bus.subscribers(), 0U);
}

} // namespace rtype::client

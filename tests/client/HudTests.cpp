#include "Hud.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <gtest/gtest.h>
#include <string>
#include <variant>
#include <vector>

#include "Actions.hpp"
#include "ClientSettings.hpp"
#include "engine/ecs/Registry.hpp"
#include "engine/ecs/Resources.hpp"
#include "engine/render/DrawCommand.hpp"
#include "engine/render/RenderQueue.hpp"
#include "GameEventBus.hpp"
#include "PresentationComponents.hpp"
#include "RenderSystems.hpp"
#include "rtype/game/Components.hpp"

namespace rtype::client {
namespace {

using engine::ecs::Registry;
using protocol::Event;
using protocol::EventType;
using protocol::PlayerInfo;
using protocol::PlayerState;

Event makeEvent(EventType type, std::uint32_t a = 0, std::uint32_t b = 0)
{
    Event event;
    event.type = type;
    event.a    = a;
    event.b    = b;
    return event;
}

void emit(GameEventBus &bus, const Event &event)
{
    bus.publish(event);
    bus.dispatch();
}

std::array<PlayerInfo, 4> makeRoster()
{
    return {{
        {.playerId     = 0,
            .state     = PlayerState::ALIVE,
            .lives     = 3,
            .score     = 1000,
            .shipNetId = 100,
            .name      = "Alice"},
        {.playerId     = 1,
            .state     = PlayerState::ALIVE,
            .lives     = 2,
            .score     = 1200,
            .shipNetId = 101,
            .name      = "Bob"},
        {.playerId     = 2,
            .state     = PlayerState::ALIVE,
            .lives     = 2,
            .score     = 1300,
            .shipNetId = 102,
            .name      = "Charlie"},
        {.playerId     = 3,
            .state     = PlayerState::ALIVE,
            .lives     = 1,
            .score     = 700,
            .shipNetId = 103,
            .name      = "David"},
    }};
}

std::vector<std::string> bannerTexts(HudReactor &reactor)
{
    std::vector<std::string> result;

    for (const Banner &banner: reactor.banners())
        result.push_back(banner.text);

    return result;
}

std::vector<engine::render::TextCmd> textCommands(const engine::render::
        RenderQueue &queue)
{
    std::vector<engine::render::TextCmd> result;

    for (const auto &command: queue.commands()) {
        if (const auto *text = std::get_if<engine::render::TextCmd>(&command)) {
            result.push_back(*text);
        }
    }
    return result;
}

void setupHudRegistry(engine::ecs::Registry &registry, GameEventBus &bus,
    ClientWorld &world, bool showHelp = false)
{
    registry.registerComponent<PlayerLabel>();
    registry.registerComponent<rtype::game::Transform>();

    registry.emplaceResource<HudReactor>(bus, world);

    HudState state;
    state.showHelp = showHelp;
    state.ping     = std::chrono::milliseconds{42};
    registry.setResource(state);

    ClientSettings settings;
    settings.hudFontSize = 32;
    registry.setResource(settings);

    auto &actions =
        registry.emplaceResource<engine::input::ActionMap<Action>>();
    bindDefaults(actions);

    registry.setResource(static_cast<const ClientWorld *>(&world));
    registry.setResource(engine::render::RenderQueue{});
    registry.setResource(engine::ecs::DeltaTime{0.0F});
}

} // namespace

// -----------------------------------------------------------------------------
// Player labels
// -----------------------------------------------------------------------------

TEST(HudTests, PlayerLabelUsesOneBasedPlayerNumbers)
{
    EXPECT_EQ(playerLabel(0), "P1");
    EXPECT_EQ(playerLabel(3), "P4");
}

// -----------------------------------------------------------------------------
// Slot lines
// -----------------------------------------------------------------------------

TEST(HudTests, SlotLineFormatsAlivePlayer)
{
    PlayerInfo slot{.playerId = 1,
        .state                = PlayerState::ALIVE,
        .lives                = 2,
        .score                = 1300,
        .shipNetId            = 42,
        .name                 = "Alice"};

    EXPECT_EQ(slotLine(slot), "P2 Alice  lives 2  1300");
}

TEST(HudTests, SlotLineFormatsRespawningPlayer)
{
    PlayerInfo slot{.playerId = 1,
        .state                = PlayerState::RESPAWNING,
        .lives                = 2,
        .score                = 1300,
        .shipNetId            = 0,
        .name                 = "Alice"};

    EXPECT_EQ(slotLine(slot), "P2 Alice  respawning  1300");
}

TEST(HudTests, SlotLineFormatsPlayerOut)
{
    PlayerInfo slot{.playerId = 1,
        .state                = PlayerState::PLAYER_OUT,
        .lives                = 0,
        .score                = 1300,
        .shipNetId            = 0,
        .name                 = "Alice"};

    EXPECT_EQ(slotLine(slot), "P2 Alice  out  1300");
}

TEST(HudTests, SlotLineFormatsEmptySlot)
{
    PlayerInfo slot{.playerId = 1, .state = PlayerState::EMPTY};

    EXPECT_EQ(slotLine(slot), "P2 -");
}

// -----------------------------------------------------------------------------
// Team score
// -----------------------------------------------------------------------------

TEST(HudTests, TeamScoreSumsAllFourPlayers)
{
    auto roster = makeRoster();

    EXPECT_EQ(teamScore(roster), 4200U);
}

TEST(HudTests, TeamScoreHandlesZeroScores)
{
    std::array<PlayerInfo, 4> roster{};

    roster[0].score = 100;
    roster[1].score = 200;
    roster[2].score = 0;
    roster[3].score = 50;

    EXPECT_EQ(teamScore(roster), 350U);
}

// -----------------------------------------------------------------------------
// HUD event reactor
// -----------------------------------------------------------------------------

TEST(HudReactorTests, PlayerJoinedCreatesReadableBanner)
{
    GameEventBus bus;
    ClientWorld world{SpriteCatalog{}};
    HudReactor reactor{bus, world};

    emit(bus, makeEvent(EventType::PLAYER_JOINED, 2));

    ASSERT_EQ(reactor.banners().size(), 1U);
    EXPECT_EQ(reactor.banners().back().text, "P3 joined");
}

TEST(HudReactorTests, PlayerLeftWithTimeoutShowsLostConnection)
{
    GameEventBus bus;
    ClientWorld world{SpriteCatalog{}};
    HudReactor reactor{bus, world};

    emit(bus,
        makeEvent(EventType::PLAYER_LEFT, 2,
            static_cast<std::uint32_t>(protocol::LeaveReason::TIMEOUT)));

    ASSERT_EQ(reactor.banners().size(), 1U);
    EXPECT_EQ(reactor.banners().back().text, "P3 lost connection");
}

TEST(HudReactorTests, PlayerLeftWithQuitShowsLeft)
{
    GameEventBus bus;
    ClientWorld world{SpriteCatalog{}};
    HudReactor reactor{bus, world};

    emit(bus,
        makeEvent(EventType::PLAYER_LEFT, 2,
            static_cast<std::uint32_t>(protocol::LeaveReason::QUIT)));

    ASSERT_EQ(reactor.banners().size(), 1U);
    EXPECT_EQ(reactor.banners().back().text, "P3 left");
}

TEST(HudReactorTests, PlayerLeftWithKickShowsWasKicked)
{
    GameEventBus bus;
    ClientWorld world{SpriteCatalog{}};
    HudReactor reactor{bus, world};

    emit(bus,
        makeEvent(EventType::PLAYER_LEFT, 2,
            static_cast<std::uint32_t>(protocol::LeaveReason::KICKED)));

    ASSERT_EQ(reactor.banners().size(), 1U);
    EXPECT_EQ(reactor.banners().back().text, "P3 was kicked");
}

/*
** PLAYER_KILLED carries the killer in a and the dead ship in b. The killer
** here has P1's ship NetId, so reading a would name P1 instead of P3.
*/
TEST(HudReactorTests, PlayerKilledNamesTheShipInBNotTheKillerInA)
{
    GameEventBus bus;
    ClientWorld world{SpriteCatalog{}};
    HudReactor reactor{bus, world};

    auto roster = makeRoster();
    world.applyRoster(roster);

    emit(bus, makeEvent(EventType::PLAYER_KILLED, 100, 102));

    ASSERT_EQ(reactor.banners().size(), 1U);
    EXPECT_EQ(reactor.banners().back().text, "P3 was shot down");
}

TEST(HudReactorTests, PlayerKilledWithAnUnknownShipAddsNoBanner)
{
    GameEventBus bus;
    ClientWorld world{SpriteCatalog{}};
    HudReactor reactor{bus, world};

    world.applyRoster(makeRoster());

    emit(bus, makeEvent(EventType::PLAYER_KILLED, 102, 999));

    EXPECT_TRUE(reactor.banners().empty());
}

TEST(HudReactorTests, KeepsOnlyTheThreeNewestBanners)
{
    GameEventBus bus;
    ClientWorld world{SpriteCatalog{}};
    HudReactor reactor{bus, world};

    emit(bus, makeEvent(EventType::PLAYER_JOINED, 0));
    emit(bus, makeEvent(EventType::PLAYER_JOINED, 1));
    emit(bus, makeEvent(EventType::PLAYER_JOINED, 2));
    emit(bus, makeEvent(EventType::PLAYER_JOINED, 3));
    emit(bus, makeEvent(EventType::PLAYER_JOINED, 0));

    ASSERT_EQ(reactor.banners().size(), HudReactor::MAX_BANNERS);

    const auto banners = bannerTexts(reactor);

    EXPECT_EQ(banners[0], "P3 joined");
    EXPECT_EQ(banners[1], "P4 joined");
    EXPECT_EQ(banners[2], "P1 joined");
}

TEST(HudReactorTests, BannerExpiresAfterThreeSeconds)
{
    GameEventBus bus;
    ClientWorld world{SpriteCatalog{}};
    HudReactor reactor{bus, world};

    emit(bus, makeEvent(EventType::PLAYER_JOINED, 0));

    ASSERT_EQ(reactor.banners().size(), 1U);

    reactor.update(3.1F);

    EXPECT_TRUE(reactor.banners().empty());
}

TEST(HudReactorTests, GameOverStartsFiveSecondCountdown)
{
    GameEventBus bus;
    ClientWorld world{SpriteCatalog{}};
    HudReactor reactor{bus, world};

    auto roster = makeRoster();
    world.applyRoster(roster);

    emit(bus, makeEvent(EventType::GAME_OVER));

    ASSERT_TRUE(reactor.gameOverCountdown().has_value());
    EXPECT_FLOAT_EQ(*reactor.gameOverCountdown(), 5.0F);

    ASSERT_EQ(reactor.banners().size(), 1U);
    EXPECT_EQ(reactor.banners().back().text, "GAME OVER - team score 4200");
}

TEST(HudReactorTests, GameOverCountdownDecreasesWithDeltaTime)
{
    GameEventBus bus;
    ClientWorld world{SpriteCatalog{}};
    HudReactor reactor{bus, world};

    emit(bus, makeEvent(EventType::GAME_OVER));

    reactor.update(2.0F);

    ASSERT_TRUE(reactor.gameOverCountdown().has_value());
    EXPECT_FLOAT_EQ(*reactor.gameOverCountdown(), 3.0F);
}

TEST(HudReactorTests, GameRestartedClearsGameOverCountdown)
{
    GameEventBus bus;
    ClientWorld world{SpriteCatalog{}};
    HudReactor reactor{bus, world};

    emit(bus, makeEvent(EventType::GAME_OVER));
    ASSERT_TRUE(reactor.gameOverCountdown().has_value());

    reactor.update(2.0F);
    emit(bus, makeEvent(EventType::GAME_RESTARTED));

    EXPECT_FALSE(reactor.gameOverCountdown().has_value());

    ASSERT_EQ(reactor.banners().size(), 2U);
    EXPECT_EQ(reactor.banners().back().text, "New game");
}

TEST(HudReactorTests, GameOverCountdownNeverGoesBelowZero)
{
    GameEventBus bus;
    ClientWorld world{SpriteCatalog{}};
    HudReactor reactor{bus, world};

    emit(bus, makeEvent(EventType::GAME_OVER));

    reactor.update(10.0F);

    ASSERT_TRUE(reactor.gameOverCountdown().has_value());
    EXPECT_FLOAT_EQ(*reactor.gameOverCountdown(), 0.0F);
}

// -----------------------------------------------------------------------------
// HUD rendering
// -----------------------------------------------------------------------------

TEST(HudSystemTests, EveryTextCommandUsesConfiguredFontSize)
{
    GameEventBus bus;
    ClientWorld world{SpriteCatalog{}};
    Registry registry;

    setupHudRegistry(registry, bus, world);

    hudSystem(registry);

    const auto texts =
        textCommands(registry.getResource<engine::render::RenderQueue>());

    ASSERT_FALSE(texts.empty());

    for (const auto &text: texts)
        EXPECT_EQ(text.characterSize, 32);
}

TEST(HudSystemTests, DisplaysPingWhenASampleExists)
{
    GameEventBus bus;
    ClientWorld world{SpriteCatalog{}};
    Registry registry;

    setupHudRegistry(registry, bus, world);

    hudSystem(registry);

    const auto texts =
        textCommands(registry.getResource<engine::render::RenderQueue>());

    const auto found = std::ranges::find_if(texts,
        [](const auto &text) { return text.text == "ping 42 ms"; });

    EXPECT_NE(found, texts.end());
}

TEST(HudSystemTests, DisplaysUnknownPingWithoutASample)
{
    GameEventBus bus;
    ClientWorld world{SpriteCatalog{}};
    Registry registry;

    setupHudRegistry(registry, bus, world);

    registry.getResource<HudState>().ping.reset();

    hudSystem(registry);

    const auto texts =
        textCommands(registry.getResource<engine::render::RenderQueue>());

    const auto found = std::ranges::find_if(texts,
        [](const auto &text) { return text.text == "ping -"; });

    EXPECT_NE(found, texts.end());
}

/*
** Raylib's default font draws '?' for any codepoint it lacks, such as the
** em and en dashes: every HUD text must stay printable ASCII. The frame has
** empty slots, no ping, a game over and the help, so every string is used.
*/
TEST(HudSystemTests, EveryTextIsPrintableAscii)
{
    GameEventBus bus;
    ClientWorld world{SpriteCatalog{}};
    Registry registry;

    setupHudRegistry(registry, bus, world, true);
    registry.getResource<HudState>().ping.reset();
    emit(bus, makeEvent(EventType::GAME_OVER));

    hudSystem(registry);

    const auto texts =
        textCommands(registry.getResource<engine::render::RenderQueue>());

    ASSERT_FALSE(texts.empty());
    for (const auto &text: texts) {
        const bool ascii = std::ranges::all_of(text.text, [](char character) {
            return character >= ' ' && character <= '~';
        });

        EXPECT_TRUE(ascii) << "Not printable ASCII: " << text.text;
    }
}

TEST(HudSystemTests, ShowHelpDisplaysEveryActionAndItsKeys)
{
    GameEventBus bus;
    ClientWorld world{SpriteCatalog{}};
    Registry registry;

    setupHudRegistry(registry, bus, world, true);

    hudSystem(registry);

    const auto texts =
        textCommands(registry.getResource<engine::render::RenderQueue>());

    constexpr std::array ACTIONS{Action::MOVE_UP, Action::MOVE_DOWN,
        Action::MOVE_LEFT, Action::MOVE_RIGHT, Action::FIRE,
        Action::TOGGLE_HUD_HELP, Action::QUIT, Action::CONFIRM};

    for (const Action action: ACTIONS) {
        const std::string prefix = std::string(toString(action)) + ": ";

        const auto count =
            std::ranges::count_if(texts, [&prefix](const auto &text) {
                return text.text.starts_with(prefix);
            });

        EXPECT_EQ(count, 1) << "Missing or duplicated help entry: " << prefix;
    }

    const auto moveUp = std::ranges::find_if(texts,
        [](const auto &text) { return text.text.starts_with("MOVE_UP: "); });

    ASSERT_NE(moveUp, texts.end());
    EXPECT_NE(moveUp->text.find("UP"), std::string::npos);
    EXPECT_NE(moveUp->text.find("W"), std::string::npos);
}

/*
** At hudFontSize 32 a fixed help position overlapped the 4th slot line: the
** help must start below the bottom of the last slot line, whatever the size.
*/
TEST(HudSystemTests, TheHelpStartsBelowTheSlotLinesAtFontSize32)
{
    GameEventBus bus;
    ClientWorld world{SpriteCatalog{}};
    Registry registry;

    setupHudRegistry(registry, bus, world, true);

    hudSystem(registry);

    const auto texts =
        textCommands(registry.getResource<engine::render::RenderQueue>());
    float slotsBottom = 0.0F;
    float helpTop     = SCREEN_HEIGHT;

    for (const auto &text: texts) {
        const float top = text.position.y();

        if (text.text.starts_with("P") &&
            text.position.x() == slotTextX(text.characterSize))
            slotsBottom = std::max(slotsBottom,
                top + static_cast<float>(text.characterSize));
        if (text.text.find(": ") != std::string::npos)
            helpTop = std::min(helpTop, top);
    }
    ASSERT_GT(slotsBottom, 0.0F);
    EXPECT_GE(helpTop, slotsBottom);
}

// -----------------------------------------------------------------------------
// Health bars
// -----------------------------------------------------------------------------

TEST(HudTests, HealthFractionIsHealthOverMaxForALiveShip)
{
    EXPECT_FLOAT_EQ(healthFraction({.state = PlayerState::ALIVE,
                        .health                 = 25,
                        .maxHealth              = 100}),
        0.25F);
    EXPECT_FLOAT_EQ(healthFraction({.state = PlayerState::RESPAWNING,
                        .maxHealth              = 100}),
        0.0F);
    EXPECT_FLOAT_EQ(healthFraction({.state = PlayerState::ALIVE}), 0.0F);
}

/*
** At size 32 a bar is 128 wide. A full ship gets a frame and a full green
** fill, a ship at 20% a frame and a red fifth, a respawning player only the
** frame, and an EMPTY slot no bar.
*/
TEST(HudSystemTests, EachJoinedSlotHasAHealthBarFilledToItsHealth)
{
    GameEventBus bus;
    ClientWorld world{SpriteCatalog{}};
    Registry registry;
    auto roster = makeRoster();

    roster[0].health    = 100;
    roster[0].maxHealth = 100;
    roster[1].health    = 20;
    roster[1].maxHealth = 100;
    roster[2].state     = PlayerState::RESPAWNING;
    roster[2].maxHealth = 100;
    roster[3]           = {.playerId = 3};
    world.applyRoster(roster);
    setupHudRegistry(registry, bus, world);

    hudSystem(registry);

    std::vector<engine::render::RectCmd> rects;
    for (const auto &command:
        registry.getResource<engine::render::RenderQueue>().commands())
        if (const auto *rect = std::get_if<engine::render::RectCmd>(&command))
            rects.push_back(*rect);

    ASSERT_EQ(rects.size(), 5U);
    EXPECT_FLOAT_EQ(rects[0].rect.width(), 128.0F);
    EXPECT_FLOAT_EQ(rects[1].rect.width(), 128.0F);
    EXPECT_EQ(rects[1].fill, engine::core::Color(60, 200, 80, 255));
    EXPECT_FLOAT_EQ(rects[3].rect.width(), 128.0F * 0.2F);
    EXPECT_EQ(rects[3].fill, engine::core::Color(220, 60, 60, 255));
    EXPECT_FLOAT_EQ(rects[4].rect.width(), 128.0F);
    EXPECT_GT(rects[4].rect.origin().y(), rects[3].rect.origin().y());
    for (const auto &rect: rects)
        EXPECT_FLOAT_EQ(rect.rect.origin().x(), MARGIN);
}

TEST(HudSystemTests, SlotTextStartsRightOfTheBar)
{
    GameEventBus bus;
    ClientWorld world{SpriteCatalog{}};
    Registry registry;

    world.applyRoster(makeRoster());
    setupHudRegistry(registry, bus, world);

    hudSystem(registry);

    const auto texts =
        textCommands(registry.getResource<engine::render::RenderQueue>());
    const auto alice = std::ranges::find_if(texts,
        [](const auto &text) { return text.text.starts_with("P1 Alice"); });

    ASSERT_NE(alice, texts.end());
    EXPECT_FLOAT_EQ(alice->position.x(),
        MARGIN + ((HEALTH_BAR_WIDTH + HEALTH_BAR_GAP) * 32.0F));
}

} // namespace rtype::client

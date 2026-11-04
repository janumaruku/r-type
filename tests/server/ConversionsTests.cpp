/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ConversionsTests
*/

#include "Conversions.hpp"

#include <gtest/gtest.h>

#include <array>
#include <cstdint>
#include <string>
#include <variant>

namespace {

namespace game = rtype::game;
namespace proto = rtype::protocol;
namespace server = rtype::server;

template <typename Message>
void expectDeserializes(const Message &message, const Message &expected)
{
    net::Packet packet;
    proto::serialize(packet, message);
    Message decoded;
    proto::deserialize(packet, decoded);
    EXPECT_EQ(decoded, expected);
    EXPECT_TRUE(packet.fullyRead());
}

}

TEST(ConversionsTests, StaticAssertionsCompile)
{
    SUCCEED();
}

TEST(ConversionsTests, ConvertsEveryGameEventType)
{
    const std::array<std::pair<game::GameEvent, proto::Event>, 8> cases{
        std::pair{game::GameEvent{game::Fired{.shooter=101, .missile=501, .side=game::Side::PLAYER,
                                   .position={1.5F, 2.5F}}},
            proto::Event{.type = proto::EventType::PLAYER_FIRED, .a = 101,
                .b = 501, .x = 1.5F, .y = 2.5F}},
        std::pair{game::GameEvent{game::Fired{.shooter=102, .missile=502, .side=game::Side::ENEMY,
                                   .position={3.5F, 4.5F}}},
            proto::Event{.type = proto::EventType::ENEMY_FIRED, .a = 102,
                .b = 502, .x = 3.5F, .y = 4.5F}},
        std::pair{game::GameEvent{game::EnemyKilled{.killerShip=103, .enemy=503, .position={5.5F, 6.5F}}},
            proto::Event{.type = proto::EventType::ENEMY_KILLED, .a = 103,
                .b = 503, .x = 5.5F, .y = 6.5F}},
        std::pair{game::GameEvent{game::PlayerKilled{.killer=104, .ship=504, .playerId=1,
                                   .position={7.5F, 8.5F}}},
            proto::Event{.type = proto::EventType::PLAYER_KILLED, .a = 104,
                .b = 504, .x = 7.5F, .y = 8.5F}},
        std::pair{game::GameEvent{game::PlayerJoined{2}},
            proto::Event{.type = proto::EventType::PLAYER_JOINED, .a = 2}},
        std::pair{game::GameEvent{game::PlayerLeft{.playerId=3, .reason=game::LeaveReason::KICKED}},
            proto::Event{.type = proto::EventType::PLAYER_LEFT, .a = 3,
                .b = 2}},
        std::pair{game::GameEvent{game::GameOver{12345}},
            proto::Event{.type = proto::EventType::GAME_OVER, .a = 12345}},
        std::pair{game::GameEvent{game::GameRestarted{}},
            proto::Event{.type = proto::EventType::GAME_RESTARTED}},
    };

    for (const auto &[input, expected] : cases) {
        const auto output = server::toProtocol(input);
        EXPECT_EQ(output.event, expected);
        EXPECT_EQ(output.copies,
            expected.type == proto::EventType::PLAYER_FIRED
                    || expected.type == proto::EventType::ENEMY_FIRED
                    || expected.type == proto::EventType::ENEMY_KILLED
                ? 1
                : 3);
        expectDeserializes(output.event, expected);
    }
}

TEST(ConversionsTests, ConvertsFiredForBothSides)
{
    const auto player = server::toProtocol(game::GameEvent{
        game::Fired{.shooter=1, .missile=2, .side=game::Side::PLAYER, .position={10.0F, 20.0F}}});
    const auto enemy = server::toProtocol(game::GameEvent{
        game::Fired{.shooter=3, .missile=4, .side=game::Side::ENEMY, .position={30.0F, 40.0F}}});

    EXPECT_EQ(player.event.type, proto::EventType::PLAYER_FIRED);
    EXPECT_EQ(enemy.event.type, proto::EventType::ENEMY_FIRED);
}

TEST(ConversionsTests, ConvertsRosterAndEveryEntryDeserializes)
{
    game::PlayerRoster roster;
    ASSERT_EQ(roster.reserve(), 0);
    ASSERT_EQ(roster.reserve(), 1);
    ASSERT_EQ(roster.reserve(), 2);
    ASSERT_TRUE(roster.join(0, "Alice", 3));
    ASSERT_TRUE(roster.onShipSpawned(0, 100));
    ASSERT_TRUE(roster.join(1, "Bob", 2));

    const auto converted = server::rosterToProtocol(roster);
    const std::array expectedStates{
        proto::PlayerState::ALIVE, proto::PlayerState::RESPAWNING,
        proto::PlayerState::EMPTY, proto::PlayerState::EMPTY};
    for (std::size_t i = 0; i < converted.size(); ++i) {
        EXPECT_EQ(converted[i].state, expectedStates[i]);
        expectDeserializes(converted[i], converted[i]);
    }
}

TEST(ConversionsTests, ConvertsEveryReplicatedEntityKindAndDeserializes)
{
    const std::array kinds{
        game::EntityKind::PLAYER_SHIP, game::EntityKind::BYDOS_SLAVE,
        game::EntityKind::PLAYER_MISSILE, game::EntityKind::ENEMY_MISSILE};
    for (std::size_t i = 0; i < kinds.size(); ++i) {
        const game::ReplicatedEntity input{
            .netId = static_cast<std::uint32_t>(101 + i),
            .kind = kinds[i],
            .variant = static_cast<std::uint8_t>(
                kinds[i] == game::EntityKind::PLAYER_SHIP ? 2 : 0),
            .position = {1.0F + static_cast<float>(i), 2.0F},
            .velocity = {3.0F, 4.0F + static_cast<float>(i)},
        };
        const auto converted = server::toProtocol(input);
        const proto::EntityState expected{
            .netId = input.netId,
            .kind = static_cast<proto::EntityKind>(input.kind),
            .variant = input.variant,
            .x = input.position.x(),
            .y = input.position.y(),
            .vx = input.velocity.x(),
            .vy = input.velocity.y(),
        };
        EXPECT_EQ(converted, expected);
        expectDeserializes(converted, expected);
    }
}

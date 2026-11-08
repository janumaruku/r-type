/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** FixedMessageTests
*/

#include <cstdint>
#include <gtest/gtest.h>
#include <limits>
#include <string>
#include <vector>

#include "MessageTestHelpers.hpp"
#include "rtype/protocol/Messages.hpp"
#include "rtype/protocol/ProtocolError.hpp"

namespace proto = rtype::protocol;
using protocol_test::encode;
using protocol_test::expectEveryPrefixTruncated;
using protocol_test::expectOutOfRange;
using protocol_test::expectRoundTrip;

namespace {

const std::string SIXTEEN_BYTES = "ABCDEFGHIJKLMNOP";

constexpr std::uint8_t ALL_BUTTONS = proto::buttons::UP | proto::buttons::DOWN |
    proto::buttons::LEFT | proto::buttons::RIGHT | proto::buttons::FIRE;

} // namespace

TEST(FixedMessageTests, EveryMessageRoundTrips)
{
    expectRoundTrip(proto::Hello{
        .protocolVersion = 1, .playerName = SIXTEEN_BYTES});
    expectRoundTrip(proto::Welcome{.playerId = 3,
        .tickRate                            = 120,
        .snapshotRate                        = 20,
        .maxPlayers                          = 2,
        .serverTick                          = 0xFFFFFFFF,
        .playerSpeed                         = 450.5F,
        .playerHalfWidth                     = 20.0F,
        .playerHalfHeight                    = 9.5F});
    expectRoundTrip(proto::Reject{.reason = proto::RejectReason::SERVER_FULL});
    expectRoundTrip(proto::Reject{
        .reason = proto::RejectReason::VERSION_MISMATCH});
    expectRoundTrip(proto::Input{
        .inputSequence = 0xFFFFFFFF, .buttons = ALL_BUTTONS});
    expectRoundTrip(proto::Disconnect{.reason = proto::DisconnectReason::QUIT});
    expectRoundTrip(proto::Shutdown{});
    expectRoundTrip(proto::Kick{.reason = proto::KickReason::PROTOCOL_ERROR});
    expectRoundTrip(proto::Kick{.reason = proto::KickReason::RATE_LIMITED});
}

TEST(FixedMessageTests, TheRfcInputPayloadIsWrittenByteForByte)
{
    const std::vector<std::uint8_t> expected{0x2C, 0x03, 0x00, 0x00, 0x11};

    EXPECT_EQ(encode(proto::Input{.inputSequence = 812,
                  .buttons = proto::buttons::UP | proto::buttons::FIRE}),
        expected);
}

TEST(FixedMessageTests, PayloadSizesMatchTheRfc)
{
    EXPECT_EQ(encode(proto::Hello{}).size(), 2U);
    EXPECT_EQ(encode(proto::Hello{.playerName = SIXTEEN_BYTES}).size(), 18U);
    EXPECT_EQ(encode(proto::Welcome{}).size(), 20U);
    EXPECT_EQ(encode(proto::Reject{}).size(), 1U);
    EXPECT_EQ(encode(proto::Input{}).size(), 5U);
    EXPECT_EQ(encode(proto::Disconnect{}).size(), 1U);
    EXPECT_EQ(encode(proto::Shutdown{}).size(), 0U);
    EXPECT_EQ(encode(proto::Kick{}).size(), 1U);
}

TEST(FixedMessageTests, EveryPrefixOfAPayloadIsTruncated)
{
    expectEveryPrefixTruncated(proto::Hello{.playerName = "Pilot"});
    expectEveryPrefixTruncated(proto::Welcome{});
    expectEveryPrefixTruncated(proto::Reject{});
    expectEveryPrefixTruncated(proto::Input{
        .inputSequence = 812, .buttons = proto::buttons::FIRE});
    expectEveryPrefixTruncated(proto::Disconnect{});
    expectEveryPrefixTruncated(proto::Kick{});
}

TEST(FixedMessageTests, AHelloNameLongerThanSixteenBytesIsRejected)
{
    expectOutOfRange(proto::Hello{.playerName = SIXTEEN_BYTES + "Q"});
}

TEST(FixedMessageTests, AHelloIsDecodedWhateverItsVersion)
{
    expectRoundTrip(proto::Hello{.protocolVersion = 2, .playerName = "Pilot"});
}

TEST(FixedMessageTests, AHelloNameWithAControlByteIsDecoded)
{
    expectRoundTrip(proto::Hello{.playerName = std::string("Ab\x01"
                                                           "c")});
}

TEST(FixedMessageTests, AnInvalidWelcomeIsRejected)
{
    expectOutOfRange(proto::Welcome{.playerId = 4});
    expectOutOfRange(proto::Welcome{.maxPlayers = 0});
    expectOutOfRange(proto::Welcome{.maxPlayers = 5});
    expectOutOfRange(proto::Welcome{.tickRate = 0});
    expectOutOfRange(proto::Welcome{.snapshotRate = 0});
}

/*
** The client predicts its ship with the movement rules: a value <= 0, an
** infinity or a NaN would freeze it or throw it out of bounds.
*/
TEST(FixedMessageTests, AWelcomeWithABadMovementRuleIsRejected)
{
    constexpr float NOT_A_NUMBER = std::numeric_limits<float>::quiet_NaN();
    constexpr float INFINITE     = std::numeric_limits<float>::infinity();

    expectOutOfRange(proto::Welcome{.playerSpeed = 0.0F});
    expectOutOfRange(proto::Welcome{.playerSpeed = NOT_A_NUMBER});
    expectOutOfRange(proto::Welcome{.playerSpeed = INFINITE});
    expectOutOfRange(proto::Welcome{.playerHalfWidth = -16.0F});
    expectOutOfRange(proto::Welcome{.playerHalfWidth = 0.0F});
    expectOutOfRange(proto::Welcome{.playerHalfHeight = NOT_A_NUMBER});
}

TEST(FixedMessageTests, AnUnknownRejectReasonIsRejected)
{
    expectOutOfRange(proto::Reject{
        .reason = static_cast<proto::RejectReason>(0)});
    expectOutOfRange(proto::Reject{
        .reason = static_cast<proto::RejectReason>(3)});
}

TEST(FixedMessageTests, AnInputWithAReservedBitIsRejected)
{
    for (unsigned bit = 5; bit <= 7; ++bit) {
        SCOPED_TRACE("bit " + std::to_string(bit));
        expectOutOfRange(proto::Input{
            .buttons = static_cast<std::uint8_t>(1U << bit)});
    }
}

TEST(FixedMessageTests, AnUnknownDisconnectReasonIsRejected)
{
    expectOutOfRange(proto::Disconnect{
        .reason = static_cast<proto::DisconnectReason>(1)});
}

TEST(FixedMessageTests, AnUnknownKickReasonIsRejected)
{
    expectOutOfRange(proto::Kick{.reason = static_cast<proto::KickReason>(0)});
    expectOutOfRange(proto::Kick{.reason = static_cast<proto::KickReason>(3)});
}

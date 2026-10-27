/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** FixedMessageTests
*/

#include <gtest/gtest.h>

#include <cstdint>
#include <string>
#include <vector>

#include "MessageTestHelpers.hpp"
#include "rtype/protocol/Messages.hpp"
#include "rtype/protocol/ProtocolError.hpp"

using namespace rtype::protocol;
using namespace protocol_test;

namespace {

const std::string SIXTEEN_BYTES = "ABCDEFGHIJKLMNOP";

constexpr std::uint8_t ALL_BUTTONS = buttons::UP | buttons::DOWN
    | buttons::LEFT | buttons::RIGHT | buttons::FIRE;

}

TEST(FixedMessageTests, EveryMessageRoundTrips)
{
    expectRoundTrip(Hello{.protocolVersion = 1, .playerName = SIXTEEN_BYTES});
    expectRoundTrip(Welcome{.playerId = 3, .tickRate = 120, .snapshotRate = 20,
        .maxPlayers = 2, .serverTick = 0xFFFFFFFF});
    expectRoundTrip(Reject{.reason = RejectReason::SERVER_FULL});
    expectRoundTrip(Reject{.reason = RejectReason::VERSION_MISMATCH});
    expectRoundTrip(Input{.inputSequence = 0xFFFFFFFF, .buttons = ALL_BUTTONS});
    expectRoundTrip(Disconnect{.reason = DisconnectReason::QUIT});
    expectRoundTrip(Shutdown{});
    expectRoundTrip(Kick{.reason = KickReason::PROTOCOL_ERROR});
    expectRoundTrip(Kick{.reason = KickReason::RATE_LIMITED});
}

TEST(FixedMessageTests, TheRfcInputPayloadIsWrittenByteForByte)
{
    const std::vector<std::uint8_t> expected{0x2C, 0x03, 0x00, 0x00, 0x11};

    EXPECT_EQ(encode(Input{.inputSequence = 812,
        .buttons = buttons::UP | buttons::FIRE}), expected);
}

TEST(FixedMessageTests, PayloadSizesMatchTheRfc)
{
    EXPECT_EQ(encode(Hello{}).size(), 2U);
    EXPECT_EQ(encode(Hello{.playerName = SIXTEEN_BYTES}).size(), 18U);
    EXPECT_EQ(encode(Welcome{}).size(), 8U);
    EXPECT_EQ(encode(Reject{}).size(), 1U);
    EXPECT_EQ(encode(Input{}).size(), 5U);
    EXPECT_EQ(encode(Disconnect{}).size(), 1U);
    EXPECT_EQ(encode(Shutdown{}).size(), 0U);
    EXPECT_EQ(encode(Kick{}).size(), 1U);
}

TEST(FixedMessageTests, EveryPrefixOfAPayloadIsTruncated)
{
    expectEveryPrefixTruncated(Hello{.playerName = "Pilot"});
    expectEveryPrefixTruncated(Welcome{});
    expectEveryPrefixTruncated(Reject{});
    expectEveryPrefixTruncated(Input{.inputSequence = 812, .buttons = buttons::FIRE});
    expectEveryPrefixTruncated(Disconnect{});
    expectEveryPrefixTruncated(Kick{});
}

TEST(FixedMessageTests, AHelloNameLongerThanSixteenBytesIsRejected)
{
    expectOutOfRange(Hello{.playerName = SIXTEEN_BYTES + "Q"});
}

TEST(FixedMessageTests, AHelloIsDecodedWhateverItsVersion)
{
    expectRoundTrip(Hello{.protocolVersion = 2, .playerName = "Pilot"});
}

TEST(FixedMessageTests, AHelloNameWithAControlByteIsDecoded)
{
    expectRoundTrip(Hello{.playerName = std::string("Ab\x01" "c")});
}

TEST(FixedMessageTests, AnInvalidWelcomeIsRejected)
{
    expectOutOfRange(Welcome{.playerId = 4});
    expectOutOfRange(Welcome{.maxPlayers = 0});
    expectOutOfRange(Welcome{.maxPlayers = 5});
    expectOutOfRange(Welcome{.tickRate = 0});
    expectOutOfRange(Welcome{.snapshotRate = 0});
}

TEST(FixedMessageTests, AnUnknownRejectReasonIsRejected)
{
    expectOutOfRange(Reject{.reason = static_cast<RejectReason>(0)});
    expectOutOfRange(Reject{.reason = static_cast<RejectReason>(3)});
}

TEST(FixedMessageTests, AnInputWithAReservedBitIsRejected)
{
    for (unsigned bit = 5; bit <= 7; ++bit) {
        SCOPED_TRACE("bit " + std::to_string(bit));
        expectOutOfRange(Input{.buttons = static_cast<std::uint8_t>(1U << bit)});
    }
}

TEST(FixedMessageTests, AnUnknownDisconnectReasonIsRejected)
{
    expectOutOfRange(Disconnect{.reason = static_cast<DisconnectReason>(1)});
}

TEST(FixedMessageTests, AnUnknownKickReasonIsRejected)
{
    expectOutOfRange(Kick{.reason = static_cast<KickReason>(0)});
    expectOutOfRange(Kick{.reason = static_cast<KickReason>(3)});
}

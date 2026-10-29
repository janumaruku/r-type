/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** MalformedInputTests
*/

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <random>
#include <span>
#include <sstream>
#include <string>
#include <vector>

#include "MessageTestHelpers.hpp"
#include "rtype/protocol/Decode.hpp"
#include "rtype/protocol/Event.hpp"
#include "rtype/protocol/Header.hpp"
#include "rtype/protocol/Messages.hpp"
#include "rtype/protocol/ProtocolError.hpp"
#include "rtype/protocol/Snapshot.hpp"

namespace proto = rtype::protocol;
using protocol_test::encode;

namespace {

using Bytes = std::vector<std::uint8_t>;

constexpr std::array<std::uint8_t, 4> EDGE_VALUES{0x00, 0x7F, 0x80, 0xFF};

constexpr int RANDOM_DATAGRAMS = 1000;

constexpr std::array EVERY_TYPE{proto::MessageType::HELLO,
    proto::MessageType::INPUT,
    proto::MessageType::DISCONNECT, proto::MessageType::WELCOME,
        proto::MessageType::REJECT,
    proto::MessageType::SNAPSHOT, proto::MessageType::EVENT,
        proto::MessageType::SHUTDOWN,
    proto::MessageType::KICK};

void decodePayload(proto::MessageType type, net::Packet &packet)
{
    switch (type) {
    case proto::MessageType::HELLO:
        static_cast<void>(proto::decode<proto::Hello>(packet));
        return;
    case proto::MessageType::INPUT:
        static_cast<void>(proto::decode<proto::Input>(packet));
        return;
    case proto::MessageType::DISCONNECT:
        static_cast<void>(proto::decode<proto::Disconnect>(packet));
        return;
    case proto::MessageType::WELCOME:
        static_cast<void>(proto::decode<proto::Welcome>(packet));
        return;
    case proto::MessageType::REJECT:
        static_cast<void>(proto::decode<proto::Reject>(packet));
        return;
    case proto::MessageType::SNAPSHOT:
        static_cast<void>(proto::decode<proto::Snapshot>(packet));
        return;
    case proto::MessageType::EVENT:
        static_cast<void>(proto::decode<proto::Event>(packet));
        return;
    case proto::MessageType::SHUTDOWN:
        static_cast<void>(proto::decode<proto::Shutdown>(packet));
        return;
    case proto::MessageType::KICK:
        static_cast<void>(proto::decode<proto::Kick>(packet));
        return;
    }
}

/*
** The dispatch order every receiver follows (RFC "Error handling"). Another
** version is dropped without an error, as the REJECT it may cause belongs to
** the server.
*/
void dispatch(std::span<const std::uint8_t> datagram,
    proto::Direction direction)
{
    proto::checkDatagramSize(datagram.size());
    net::Packet packet{datagram};
    const auto header = packet.read<proto::Header>();

    if (header.version != proto::Header::VERSION)
        return;
    proto::checkType(header.type, direction);
    decodePayload(header.type, packet);
}

/*
** The code the first failing check gives, or nothing if the datagram passes.
** Any exception other than a net::PacketError escapes and fails the test.
*/
template <class Function>
std::optional<proto::DecodeError> failureOf(Function &&function)
{
    try {
        function();
    } catch (const net::PacketError &error) {
        return proto::classify(error);
    }
    return std::nullopt;
}

template <class Message>
Bytes datagram(const Message &message)
{
    const net::Packet packet = proto::makePacket(Message::TYPE, 1, message);
    const auto bytes = packet.bytes();

    return {bytes.begin(), bytes.end()};
}

template <class Message>
std::optional<proto::DecodeError> decodeFailureOf(
    std::span<const std::uint8_t> payload)
{
    return failureOf([payload] {
        net::Packet packet{payload};
        static_cast<void>(proto::decode<Message>(packet));
    });
}

template <class... Messages>
void decodeAsEach(std::span<const std::uint8_t> payload)
{
    (static_cast<void>(decodeFailureOf<Messages>(payload)), ...);
}

void decodeAsEveryType(std::span<const std::uint8_t> payload)
{
    decodeAsEach<proto::Hello, proto::Input, proto::Disconnect, proto::Welcome,
        proto::Reject, proto::Snapshot, proto::Event,
        proto::Shutdown, proto::Kick>(payload);
}

proto::Snapshot smallSnapshot()
{
    proto::Snapshot snapshot{.serverTick = 7, .lastProcessedInput = 3};

    for (std::uint8_t slot = 0; slot < proto::MAX_PLAYERS; ++slot)
        snapshot.roster.push_back(proto::PlayerInfo{.playerId = slot,
            .state = proto::PlayerState::ALIVE, .lives = 3,
                .shipNetId = 10U + slot,
            .name = "P" + std::to_string(slot)});
    snapshot.entities.push_back(proto::EntityState{.netId = 10,
        .kind = proto::EntityKind::PLAYER_SHIP, .x = 1.0F});
    snapshot.entities.push_back(proto::EntityState{.netId = 20,
        .kind = proto::EntityKind::BYDOS_SLAVE,
            .variant = proto::pattern::SINE});
    return snapshot;
}

/*
** The largest valid payload: 4 players with 16-byte names and 48 entities.
*/
proto::Snapshot fullSnapshot()
{
    proto::Snapshot snapshot = smallSnapshot();

    for (proto::PlayerInfo &player: snapshot.roster)
        player.name = "ABCDEFGHIJKLMNOP";
    snapshot.entities.clear();
    for (std::uint32_t netId = 1; netId <= proto::MAX_ENTITIES_PER_CHUNK;
        ++netId)
        snapshot.entities.push_back(proto::EntityState{.netId = netId,
            .kind = proto::EntityKind::ENEMY_MISSILE, .vx = -300.0F});
    return snapshot;
}

template <class Message>
void expectDecoded(const Message &message)
{
    SCOPED_TRACE(::testing::PrintToString(Message::TYPE));
    const Bytes bytes = encode(message);
    net::Packet packet{bytes};

    EXPECT_EQ(proto::decode<Message>(packet), message);
}

template <class Message>
void expectTrailingBytes(const Message &message)
{
    std::ostringstream expected;
    Bytes bytes = encode(message);

    expected << Message::TYPE << ": bytes left over";
    bytes.push_back(0);
    net::Packet packet{bytes};
    try {
        static_cast<void>(proto::decode<Message>(packet));
        ADD_FAILURE() << Message::TYPE << " with an extra byte was accepted";
    } catch (const proto::ProtocolError &error) {
        EXPECT_EQ(error.code(),
            proto::DecodeError::TRAILING_BYTES) << Message::TYPE;
        EXPECT_EQ(std::string(error.what()), expected.str());
    }
}

/*
** Replaces each byte in turn by the values at the edges of the signed and
** unsigned ranges, and decodes the result as Message.
*/
template <class Message>
void expectEveryMutationDecodesOrFailsCleanly(const Message &message)
{
    const Bytes valid = encode(message);

    for (std::size_t index = 0; index < valid.size(); ++index) {
        for (const std::uint8_t value: EDGE_VALUES) {
            Bytes mutated = valid;

            mutated[index] = value;
            ASSERT_NO_THROW(static_cast<
                void>(decodeFailureOf<Message>(mutated)))
                << Message::TYPE << ", byte " << index << " set to "
                << static_cast<unsigned>(value);
        }
    }
}

}

TEST(MalformedInputTests, DecodeReturnsAMessageWithNothingLeftOver)
{
    expectDecoded(proto::Hello{.playerName = "Bob"});
    expectDecoded(proto::Input{.inputSequence = 9,
        .buttons = proto::buttons::FIRE});
    expectDecoded(proto::Disconnect{});
    expectDecoded(proto::Welcome{.playerId = 2});
    expectDecoded(proto::Reject{
        .reason = proto::RejectReason::VERSION_MISMATCH});
    expectDecoded(smallSnapshot());
    expectDecoded(proto::Event{.eventId = 4,
        .type = proto::EventType::GAME_OVER});
    expectDecoded(proto::Shutdown{});
    expectDecoded(proto::Kick{.reason = proto::KickReason::RATE_LIMITED});
}

TEST(MalformedInputTests, AnExtraByteAfterAnyMessageIsTrailingBytes)
{
    expectTrailingBytes(proto::Hello{.playerName = "Bob"});
    expectTrailingBytes(proto::Input{.inputSequence = 9,
        .buttons = proto::buttons::FIRE});
    expectTrailingBytes(proto::Disconnect{});
    expectTrailingBytes(proto::Welcome{.playerId = 2});
    expectTrailingBytes(proto::Reject{
        .reason = proto::RejectReason::VERSION_MISMATCH});
    expectTrailingBytes(smallSnapshot());
    expectTrailingBytes(proto::Event{.eventId = 4,
        .type = proto::EventType::GAME_OVER});
    expectTrailingBytes(proto::Shutdown{});
    expectTrailingBytes(proto::Kick{.reason = proto::KickReason::RATE_LIMITED});
}

TEST(MalformedInputTests, DecodeReadsThePayloadAfterTheHeader)
{
    const Bytes bytes = datagram(proto::Input{.inputSequence = 9,
        .buttons = 1});
    net::Packet packet{bytes};

    EXPECT_EQ(packet.read<proto::Header>().type, proto::MessageType::INPUT);
    EXPECT_EQ(proto::decode<proto::Input>(packet),
        (proto::Input{.inputSequence = 9, .buttons = 1}));
}

TEST(MalformedInputTests, AValidDatagramPassesTheWholeDispatch)
{
    EXPECT_EQ(failureOf([] {
        dispatch(datagram(proto::Input{.inputSequence = 1}),
            proto::Direction::CLIENT_TO_SERVER);
    }), std::nullopt);
    EXPECT_EQ(failureOf([] {
        dispatch(datagram(fullSnapshot()), proto::Direction::SERVER_TO_CLIENT);
    }), std::nullopt);
}

TEST(MalformedInputTests, ADatagramOfAWrongSizeIsBadSize)
{
    const Bytes valid = datagram(proto::Hello{});

    for (std::size_t size = 0; size < proto::Header::SIZE; ++size) {
        const Bytes shorter(valid.begin(),
            valid.begin() + static_cast<std::ptrdiff_t>(size));
        const Bytes zeros(size, 0);

        EXPECT_EQ(failureOf([&] {
            dispatch(zeros, proto::Direction::CLIENT_TO_SERVER);
        }), proto::DecodeError::BAD_SIZE) << size << " bytes";
        EXPECT_EQ(failureOf([&] {
            dispatch(shorter, proto::Direction::CLIENT_TO_SERVER);
        }), proto::DecodeError::BAD_SIZE) << size << " bytes of a HELLO";
    }
    EXPECT_EQ(failureOf([] {
        dispatch(Bytes(net::Packet::MAX_SIZE + 1, 0),
            proto::Direction::CLIENT_TO_SERVER);
    }), proto::DecodeError::BAD_SIZE);
}

/*
** Each datagram breaks two checks; the earlier one in the dispatch order is
** the one reported.
*/
TEST(MalformedInputTests, TheFirstFailingCheckInTheDispatchOrderWins)
{
    Bytes badMagicAndType = datagram(proto::Input{});
    badMagicAndType[0] = 0;
    badMagicAndType[3] = 0x42;
    EXPECT_EQ(failureOf([&] {
        dispatch(badMagicAndType, proto::Direction::CLIENT_TO_SERVER);
    }), proto::DecodeError::BAD_MAGIC);

    Bytes otherVersionAndType = datagram(proto::Input{});
    otherVersionAndType[2] =
        static_cast<std::uint8_t>(proto::Header::VERSION + 1);
    otherVersionAndType[3] = 0x42;
    EXPECT_EQ(failureOf([&] {
        dispatch(otherVersionAndType, proto::Direction::CLIENT_TO_SERVER);
    }), std::nullopt);

    Bytes unknownTypeAndShortPayload = datagram(proto::Input{});
    unknownTypeAndShortPayload[3] = 0x42;
    unknownTypeAndShortPayload.pop_back();
    EXPECT_EQ(failureOf([&] {
        dispatch(unknownTypeAndShortPayload,
            proto::Direction::CLIENT_TO_SERVER);
    }), proto::DecodeError::UNKNOWN_TYPE);

    Bytes wrongDirectionAndExtraByte = datagram(proto::Shutdown{});
    wrongDirectionAndExtraByte.push_back(0);
    EXPECT_EQ(failureOf([&] {
        dispatch(wrongDirectionAndExtraByte,
            proto::Direction::CLIENT_TO_SERVER);
    }), proto::DecodeError::WRONG_DIRECTION);

    Bytes shortPayload = datagram(proto::Input{});
    shortPayload.pop_back();
    EXPECT_EQ(failureOf([&] {
        dispatch(shortPayload, proto::Direction::CLIENT_TO_SERVER);
    }), proto::DecodeError::TRUNCATED);

    EXPECT_EQ(failureOf([] {
        dispatch(datagram(proto::Input{.buttons = proto::buttons::RESERVED}),
            proto::Direction::CLIENT_TO_SERVER);
    }), proto::DecodeError::OUT_OF_RANGE);

    Bytes extraByte = datagram(proto::Input{});
    extraByte.push_back(0);
    EXPECT_EQ(failureOf([&] {
        dispatch(extraByte, proto::Direction::CLIENT_TO_SERVER);
    }), proto::DecodeError::TRAILING_BYTES);
}

/*
** The vector's count is checked against the bytes left before anything is
** reserved, so a lying count can't make the receiver allocate 65535 entries.
*/
TEST(MalformedInputTests, ARosterCountOf65535InThirtyBytesIsTruncated)
{
    net::Packet packet;
    packet << std::uint32_t{1} << std::uint32_t{0} << std::uint8_t{0}
        << std::uint8_t{1} << std::uint16_t{0xFFFF};
    while (packet.size() < 30)
        packet << std::uint8_t{0};
    net::Packet payload{packet.bytes()};

    EXPECT_EQ(failureOf([&] {
        static_cast<void>(proto::decode<proto::Snapshot>(payload));
    }), proto::DecodeError::TRUNCATED);
}

TEST(MalformedInputTests, ANameLengthOf200WithFiveBytesLeftIsTruncated)
{
    const Bytes bytes{proto::Header::VERSION, 200, 'A', 'B', 'C', 'D', 'E'};

    EXPECT_EQ(decodeFailureOf<proto::Hello>(bytes),
        proto::DecodeError::TRUNCATED);
}

/*
** Not libFuzzer: a fixed seed, so a failure is reproduced by running the test
** again. Each datagram goes through the dispatch twice: as it is, and behind a
** valid header naming one of the 9 types, sent in its own direction, so the
** random bytes also reach decode. It is then decoded as each of the 9 types.
** Each throw costs about 100 µs under ASan, which sets RANDOM_DATAGRAMS.
*/
TEST(MalformedInputTests, RandomDatagramsNeverCrashTheDecoder)
{
    std::mt19937 random{0x52545950};
    std::uniform_int_distribution<std::size_t> size{0, net::Packet::MAX_SIZE};
    std::uniform_int_distribution<unsigned> byte{0, 0xFF};

    for (int index = 0; index < RANDOM_DATAGRAMS; ++index) {
        Bytes bytes(size(random));

        for (std::uint8_t &value: bytes)
            value = static_cast<std::uint8_t>(byte(random));
        const proto::MessageType type =
            EVERY_TYPE[static_cast<std::size_t>(index) % EVERY_TYPE.size()];
        const proto::Direction direction = proto::isClientToServer(type)
            ? proto::Direction::CLIENT_TO_SERVER
            : proto::Direction::SERVER_TO_CLIENT;
        Bytes behindValidHeader = bytes;
        if (behindValidHeader.size() >= proto::Header::SIZE) {
            behindValidHeader[0] =
                static_cast<std::uint8_t>(proto::Header::MAGIC & 0xFF);
            behindValidHeader[1] =
                static_cast<std::uint8_t>(proto::Header::MAGIC >> 8);
            behindValidHeader[2] = proto::Header::VERSION;
            behindValidHeader[3] = static_cast<std::uint8_t>(type);
        }
        ASSERT_NO_THROW({
            static_cast<void>(failureOf([&] { dispatch(bytes, direction); }));
            static_cast<void>(failureOf([&] {
                dispatch(behindValidHeader, direction);
            }));
            decodeAsEveryType(bytes);
        }) << "datagram " << index;
    }
}

TEST(MalformedInputTests, MutatedPayloadsNeverCrashTheDecoder)
{
    expectEveryMutationDecodesOrFailsCleanly(proto::Hello{.playerName = "Bob"});
    expectEveryMutationDecodesOrFailsCleanly(proto::Input{.inputSequence = 9,
        .buttons = proto::buttons::FIRE});
    expectEveryMutationDecodesOrFailsCleanly(proto::Disconnect{});
    expectEveryMutationDecodesOrFailsCleanly(proto::Welcome{.playerId = 2});
    expectEveryMutationDecodesOrFailsCleanly(proto::Reject{});
    expectEveryMutationDecodesOrFailsCleanly(smallSnapshot());
    expectEveryMutationDecodesOrFailsCleanly(proto::Event{.eventId = 4,
        .type = proto::EventType::PLAYER_LEFT, .a = 1,
        .b = static_cast<std::uint32_t>(proto::LeaveReason::KICKED)});
    expectEveryMutationDecodesOrFailsCleanly(proto::Kick{});
}

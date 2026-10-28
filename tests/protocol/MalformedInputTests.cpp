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

using namespace rtype::protocol;
using namespace protocol_test;

namespace {

using Bytes = std::vector<std::uint8_t>;

constexpr std::array<std::uint8_t, 4> EDGE_VALUES{0x00, 0x7F, 0x80, 0xFF};

constexpr int RANDOM_DATAGRAMS = 1000;

constexpr std::array EVERY_TYPE{MessageType::HELLO, MessageType::INPUT,
    MessageType::DISCONNECT, MessageType::WELCOME, MessageType::REJECT,
    MessageType::SNAPSHOT, MessageType::EVENT, MessageType::SHUTDOWN,
    MessageType::KICK};

void decodePayload(MessageType type, net::Packet &packet)
{
    switch (type) {
    case MessageType::HELLO:
        static_cast<void>(decode<Hello>(packet));
        return;
    case MessageType::INPUT:
        static_cast<void>(decode<Input>(packet));
        return;
    case MessageType::DISCONNECT:
        static_cast<void>(decode<Disconnect>(packet));
        return;
    case MessageType::WELCOME:
        static_cast<void>(decode<Welcome>(packet));
        return;
    case MessageType::REJECT:
        static_cast<void>(decode<Reject>(packet));
        return;
    case MessageType::SNAPSHOT:
        static_cast<void>(decode<Snapshot>(packet));
        return;
    case MessageType::EVENT:
        static_cast<void>(decode<Event>(packet));
        return;
    case MessageType::SHUTDOWN:
        static_cast<void>(decode<Shutdown>(packet));
        return;
    case MessageType::KICK:
        static_cast<void>(decode<Kick>(packet));
        return;
    }
}

/*
** The dispatch order every receiver follows (RFC "Error handling"). Another
** version is dropped without an error, as the REJECT it may cause belongs to
** the server.
*/
void dispatch(std::span<const std::uint8_t> datagram, Direction direction)
{
    checkDatagramSize(datagram.size());
    net::Packet packet{datagram};
    const auto header = packet.read<Header>();

    if (header.version != Header::VERSION)
        return;
    checkType(header.type, direction);
    decodePayload(header.type, packet);
}

/*
** The code the first failing check gives, or nothing if the datagram passes.
** Any exception other than a net::PacketError escapes and fails the test.
*/
template <class Function>
std::optional<DecodeError> failureOf(Function &&function)
{
    try {
        function();
    } catch (const net::PacketError &error) {
        return classify(error);
    }
    return std::nullopt;
}

template <class Message>
Bytes datagram(const Message &message)
{
    const net::Packet packet = makePacket(Message::TYPE, 1, message);
    const auto bytes = packet.bytes();

    return {bytes.begin(), bytes.end()};
}

template <class Message>
std::optional<DecodeError> decodeFailureOf(std::span<const std::uint8_t> payload)
{
    return failureOf([payload] {
        net::Packet packet{payload};
        static_cast<void>(decode<Message>(packet));
    });
}

template <class... Messages>
void decodeAsEach(std::span<const std::uint8_t> payload)
{
    (static_cast<void>(decodeFailureOf<Messages>(payload)), ...);
}

void decodeAsEveryType(std::span<const std::uint8_t> payload)
{
    decodeAsEach<Hello, Input, Disconnect, Welcome, Reject, Snapshot, Event,
        Shutdown, Kick>(payload);
}

Snapshot smallSnapshot()
{
    Snapshot snapshot{.serverTick = 7, .lastProcessedInput = 3};

    for (std::uint8_t slot = 0; slot < MAX_PLAYERS; ++slot)
        snapshot.roster.push_back(PlayerInfo{.playerId = slot,
            .state = PlayerState::ALIVE, .lives = 3, .shipNetId = 10U + slot,
            .name = "P" + std::to_string(slot)});
    snapshot.entities.push_back(EntityState{.netId = 10,
        .kind = EntityKind::PLAYER_SHIP, .x = 1.0F});
    snapshot.entities.push_back(EntityState{.netId = 20,
        .kind = EntityKind::BYDOS_SLAVE, .variant = pattern::SINE});
    return snapshot;
}

/*
** The largest valid payload: 4 players with 16-byte names and 48 entities.
*/
Snapshot fullSnapshot()
{
    Snapshot snapshot = smallSnapshot();

    for (PlayerInfo &player: snapshot.roster)
        player.name = "ABCDEFGHIJKLMNOP";
    snapshot.entities.clear();
    for (std::uint32_t netId = 1; netId <= MAX_ENTITIES_PER_CHUNK; ++netId)
        snapshot.entities.push_back(EntityState{.netId = netId,
            .kind = EntityKind::ENEMY_MISSILE, .vx = -300.0F});
    return snapshot;
}

template <class Message>
void expectDecoded(const Message &message)
{
    SCOPED_TRACE(::testing::PrintToString(Message::TYPE));
    const Bytes bytes = encode(message);
    net::Packet packet{bytes};

    EXPECT_EQ(decode<Message>(packet), message);
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
        static_cast<void>(decode<Message>(packet));
        ADD_FAILURE() << Message::TYPE << " with an extra byte was accepted";
    } catch (const ProtocolError &error) {
        EXPECT_EQ(error.code(), DecodeError::TRAILING_BYTES) << Message::TYPE;
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
            ASSERT_NO_THROW(static_cast<void>(decodeFailureOf<Message>(mutated)))
                << Message::TYPE << ", byte " << index << " set to "
                << static_cast<unsigned>(value);
        }
    }
}

}

TEST(MalformedInputTests, DecodeReturnsAMessageWithNothingLeftOver)
{
    expectDecoded(Hello{.playerName = "Bob"});
    expectDecoded(Input{.inputSequence = 9, .buttons = buttons::FIRE});
    expectDecoded(Disconnect{});
    expectDecoded(Welcome{.playerId = 2});
    expectDecoded(Reject{.reason = RejectReason::VERSION_MISMATCH});
    expectDecoded(smallSnapshot());
    expectDecoded(Event{.eventId = 4, .type = EventType::GAME_OVER});
    expectDecoded(Shutdown{});
    expectDecoded(Kick{.reason = KickReason::RATE_LIMITED});
}

TEST(MalformedInputTests, AnExtraByteAfterAnyMessageIsTrailingBytes)
{
    expectTrailingBytes(Hello{.playerName = "Bob"});
    expectTrailingBytes(Input{.inputSequence = 9, .buttons = buttons::FIRE});
    expectTrailingBytes(Disconnect{});
    expectTrailingBytes(Welcome{.playerId = 2});
    expectTrailingBytes(Reject{.reason = RejectReason::VERSION_MISMATCH});
    expectTrailingBytes(smallSnapshot());
    expectTrailingBytes(Event{.eventId = 4, .type = EventType::GAME_OVER});
    expectTrailingBytes(Shutdown{});
    expectTrailingBytes(Kick{.reason = KickReason::RATE_LIMITED});
}

TEST(MalformedInputTests, DecodeReadsThePayloadAfterTheHeader)
{
    const Bytes bytes = datagram(Input{.inputSequence = 9, .buttons = 1});
    net::Packet packet{bytes};

    EXPECT_EQ(packet.read<Header>().type, MessageType::INPUT);
    EXPECT_EQ(decode<Input>(packet), (Input{.inputSequence = 9, .buttons = 1}));
}

TEST(MalformedInputTests, AValidDatagramPassesTheWholeDispatch)
{
    EXPECT_EQ(failureOf([] {
        dispatch(datagram(Input{.inputSequence = 1}),
            Direction::CLIENT_TO_SERVER);
    }), std::nullopt);
    EXPECT_EQ(failureOf([] {
        dispatch(datagram(fullSnapshot()), Direction::SERVER_TO_CLIENT);
    }), std::nullopt);
}

TEST(MalformedInputTests, ADatagramOfAWrongSizeIsBadSize)
{
    const Bytes valid = datagram(Hello{});

    for (std::size_t size = 0; size < Header::SIZE; ++size) {
        const Bytes shorter(valid.begin(),
            valid.begin() + static_cast<std::ptrdiff_t>(size));
        const Bytes zeros(size, 0);

        EXPECT_EQ(failureOf([&] {
            dispatch(zeros, Direction::CLIENT_TO_SERVER);
        }), DecodeError::BAD_SIZE) << size << " bytes";
        EXPECT_EQ(failureOf([&] {
            dispatch(shorter, Direction::CLIENT_TO_SERVER);
        }), DecodeError::BAD_SIZE) << size << " bytes of a HELLO";
    }
    EXPECT_EQ(failureOf([] {
        dispatch(Bytes(net::Packet::MAX_SIZE + 1, 0),
            Direction::CLIENT_TO_SERVER);
    }), DecodeError::BAD_SIZE);
}

/*
** Each datagram breaks two checks; the earlier one in the dispatch order is
** the one reported.
*/
TEST(MalformedInputTests, TheFirstFailingCheckInTheDispatchOrderWins)
{
    Bytes badMagicAndType = datagram(Input{});
    badMagicAndType[0] = 0;
    badMagicAndType[3] = 0x42;
    EXPECT_EQ(failureOf([&] {
        dispatch(badMagicAndType, Direction::CLIENT_TO_SERVER);
    }), DecodeError::BAD_MAGIC);

    Bytes otherVersionAndType = datagram(Input{});
    otherVersionAndType[2] = static_cast<std::uint8_t>(Header::VERSION + 1);
    otherVersionAndType[3] = 0x42;
    EXPECT_EQ(failureOf([&] {
        dispatch(otherVersionAndType, Direction::CLIENT_TO_SERVER);
    }), std::nullopt);

    Bytes unknownTypeAndShortPayload = datagram(Input{});
    unknownTypeAndShortPayload[3] = 0x42;
    unknownTypeAndShortPayload.pop_back();
    EXPECT_EQ(failureOf([&] {
        dispatch(unknownTypeAndShortPayload, Direction::CLIENT_TO_SERVER);
    }), DecodeError::UNKNOWN_TYPE);

    Bytes wrongDirectionAndExtraByte = datagram(Shutdown{});
    wrongDirectionAndExtraByte.push_back(0);
    EXPECT_EQ(failureOf([&] {
        dispatch(wrongDirectionAndExtraByte, Direction::CLIENT_TO_SERVER);
    }), DecodeError::WRONG_DIRECTION);

    Bytes shortPayload = datagram(Input{});
    shortPayload.pop_back();
    EXPECT_EQ(failureOf([&] {
        dispatch(shortPayload, Direction::CLIENT_TO_SERVER);
    }), DecodeError::TRUNCATED);

    EXPECT_EQ(failureOf([] {
        dispatch(datagram(Input{.buttons = buttons::RESERVED}),
            Direction::CLIENT_TO_SERVER);
    }), DecodeError::OUT_OF_RANGE);

    Bytes extraByte = datagram(Input{});
    extraByte.push_back(0);
    EXPECT_EQ(failureOf([&] {
        dispatch(extraByte, Direction::CLIENT_TO_SERVER);
    }), DecodeError::TRAILING_BYTES);
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
        static_cast<void>(decode<Snapshot>(payload));
    }), DecodeError::TRUNCATED);
}

TEST(MalformedInputTests, ANameLengthOf200WithFiveBytesLeftIsTruncated)
{
    const Bytes bytes{Header::VERSION, 200, 'A', 'B', 'C', 'D', 'E'};

    EXPECT_EQ(decodeFailureOf<Hello>(bytes), DecodeError::TRUNCATED);
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
        const MessageType type = EVERY_TYPE[static_cast<std::size_t>(index)
            % EVERY_TYPE.size()];
        const Direction direction = isClientToServer(type)
            ? Direction::CLIENT_TO_SERVER : Direction::SERVER_TO_CLIENT;
        Bytes behindValidHeader = bytes;
        if (behindValidHeader.size() >= Header::SIZE) {
            behindValidHeader[0] = static_cast<std::uint8_t>(Header::MAGIC & 0xFF);
            behindValidHeader[1] = static_cast<std::uint8_t>(Header::MAGIC >> 8);
            behindValidHeader[2] = Header::VERSION;
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
    expectEveryMutationDecodesOrFailsCleanly(Hello{.playerName = "Bob"});
    expectEveryMutationDecodesOrFailsCleanly(Input{.inputSequence = 9,
        .buttons = buttons::FIRE});
    expectEveryMutationDecodesOrFailsCleanly(Disconnect{});
    expectEveryMutationDecodesOrFailsCleanly(Welcome{.playerId = 2});
    expectEveryMutationDecodesOrFailsCleanly(Reject{});
    expectEveryMutationDecodesOrFailsCleanly(smallSnapshot());
    expectEveryMutationDecodesOrFailsCleanly(Event{.eventId = 4,
        .type = EventType::PLAYER_LEFT, .a = 1,
        .b = static_cast<std::uint32_t>(LeaveReason::KICKED)});
    expectEveryMutationDecodesOrFailsCleanly(Kick{});
}

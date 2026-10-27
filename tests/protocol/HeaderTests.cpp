/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** HeaderTests
*/

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

#include "rtype/protocol/Header.hpp"
#include "rtype/protocol/ProtocolError.hpp"

using rtype::protocol::DecodeError;
using rtype::protocol::Header;
using rtype::protocol::MessageType;

namespace {

/*
** Stands in for the message structs of subtask 2, so that makePacket can be
** checked on its header alone.
*/
struct EmptyMessage {};

void serialize(net::Packet &, const EmptyMessage &)
{
}

void deserialize(net::Packet &, EmptyMessage &)
{
}

const std::vector<std::uint8_t> RFC_EXAMPLE{0x54, 0x52, 0x01, 0x02,
                                            0xDC, 0x05, 0x00, 0x00};

std::vector<std::uint8_t> toVector(std::span<const std::uint8_t> bytes)
{
    return {bytes.begin(), bytes.end()};
}

}

TEST(HeaderTests, TheRfcExampleIsWrittenByteForByte)
{
    net::Packet packet;

    packet << Header{.type = MessageType::INPUT, .sequence = 1500};
    EXPECT_EQ(toVector(packet.bytes()), RFC_EXAMPLE);
    EXPECT_EQ(packet.size(), Header::SIZE);
}

TEST(HeaderTests, TheRfcExampleIsReadBack)
{
    net::Packet packet(RFC_EXAMPLE);
    Header header;

    packet >> header;
    EXPECT_EQ(header.magic, Header::MAGIC);
    EXPECT_EQ(header.version, Header::VERSION);
    EXPECT_EQ(header.type, MessageType::INPUT);
    EXPECT_EQ(header.sequence, 1500U);
    EXPECT_TRUE(packet.fullyRead());
}

TEST(HeaderTests, AHeaderWithTheLargestSequenceRoundTrips)
{
    const Header written{.type = MessageType::SNAPSHOT,
                         .sequence = 0xFFFFFFFF};
    net::Packet out;

    out << written;

    net::Packet in(out.bytes());
    Header read;

    in >> read;
    EXPECT_EQ(read.magic, written.magic);
    EXPECT_EQ(read.version, written.version);
    EXPECT_EQ(read.type, written.type);
    EXPECT_EQ(read.sequence, written.sequence);
    EXPECT_TRUE(in.fullyRead());
}

TEST(HeaderTests, ABadMagicIsRejected)
{
    for (const std::uint16_t magic: {std::uint16_t{0x5253}, std::uint16_t{0x0000}}) {
        net::Packet out;
        out << magic << Header::VERSION << MessageType::HELLO
            << std::uint32_t{1};

        net::Packet in(out.bytes());
        Header header;

        try {
            in >> header;
            ADD_FAILURE() << "magic 0x" << std::hex << magic << " was accepted";
        } catch (const rtype::protocol::ProtocolError &error) {
            EXPECT_EQ(error.code(), DecodeError::BAD_MAGIC);
        }
    }
}

TEST(HeaderTests, AnotherVersionIsLeftToTheDispatcher)
{
    std::vector<std::uint8_t> bytes = RFC_EXAMPLE;
    bytes[2] = 2;
    net::Packet packet(bytes);
    Header header;

    EXPECT_NO_THROW(packet >> header);
    EXPECT_EQ(header.version, 2);
}

TEST(HeaderTests, AnUnknownTypeIsLeftToTheDispatcher)
{
    std::vector<std::uint8_t> bytes = RFC_EXAMPLE;
    bytes[3] = 0x42;
    net::Packet packet(bytes);
    Header header;

    EXPECT_NO_THROW(packet >> header);
    EXPECT_EQ(header.type, static_cast<MessageType>(0x42));
}

TEST(HeaderTests, EveryPrefixOfAHeaderIsTruncated)
{
    for (std::size_t length = 0; length < Header::SIZE; ++length) {
        net::Packet packet{std::span(RFC_EXAMPLE).first(length)};
        Header header;

        try {
            packet >> header;
            ADD_FAILURE() << "a prefix of " << length << " bytes was accepted";
        } catch (const net::PacketError &error) {
            EXPECT_EQ(rtype::protocol::classify(error), DecodeError::TRUNCATED)
                << "prefix of " << length << " bytes";
        }
    }
}

TEST(HeaderTests, MakePacketWritesTheHeaderThenTheMessage)
{
    const net::Packet packet = rtype::protocol::makePacket(MessageType::INPUT,
        1500, EmptyMessage{});

    EXPECT_EQ(toVector(packet.bytes()), RFC_EXAMPLE);

    net::Packet in(packet.bytes());
    Header header;
    EmptyMessage message;

    in >> header >> message;
    EXPECT_EQ(header.type, MessageType::INPUT);
    EXPECT_EQ(header.sequence, 1500U);
    EXPECT_TRUE(in.fullyRead());
}

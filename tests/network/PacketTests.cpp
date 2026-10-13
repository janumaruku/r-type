/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** PacketTests.cpp
*/

#include <gtest/gtest.h>

#include <array>
#include <cstdint>
#include <limits>

#include "Packet.hpp"

TEST(Packet, WritesAndReadsBytesIdentically)
{
    const std::array<std::uint8_t, 4> expected{0x00, 0x7f, 0x80, 0xff};
    net::Packet packet;

    packet.writeRaw(expected.data(), expected.size());

    std::array<std::uint8_t, expected.size()> actual{};
    packet.readRaw(actual.data(), actual.size());

    EXPECT_EQ(actual, expected);
}

TEST(Packet, StreamOperatorsWriteAndReadValues)
{
    constexpr std::uint16_t expectedShort = 0x1234;
    constexpr std::int32_t expectedInteger = -42;
    net::Packet packet;

    packet << expectedShort << expectedInteger;

    std::uint16_t actualShort = 0;
    std::int32_t actualInteger = 0;
    packet >> actualShort >> actualInteger;

    EXPECT_EQ(actualShort, expectedShort);
    EXPECT_EQ(actualInteger, expectedInteger);
    EXPECT_TRUE(packet.fullyRead());
}

TEST(Packet, ReadRawPastEndPreservesRemainingBytes)
{
    std::uint8_t value = 42;
    net::Packet packet;
    packet.writeRaw(&value, sizeof(value));
    const auto remaining = packet.remaining();

    EXPECT_THROW(packet.readRaw(&value, sizeof(value) + 1), net::PacketError);
    EXPECT_EQ(packet.remaining(), remaining);
}

TEST(Packet, WriteRawPastMaximumPreservesSize)
{
    const std::array<std::uint8_t, net::Packet::MAX_SIZE> bytes{};
    const std::uint8_t extra = 1;
    net::Packet packet;
    packet.writeRaw(bytes.data(), bytes.size());

    EXPECT_EQ(packet.size(), net::Packet::MAX_SIZE);
    EXPECT_THROW(packet.writeRaw(&extra, sizeof(extra)), net::PacketError);
    EXPECT_EQ(packet.size(), net::Packet::MAX_SIZE);
}

TEST(Packet, ReadRawSizeMaxThrowsWithoutWrapping)
{
    const std::uint8_t value = 42;
    std::uint8_t output = 0;
    net::Packet packet;
    packet.writeRaw(&value, sizeof(value));
    const auto remaining = packet.remaining();

    EXPECT_THROW(packet.readRaw(&output, std::numeric_limits<std::size_t>::max()),
                 net::PacketError);
    EXPECT_EQ(packet.remaining(), remaining);
}

TEST(Packet, SpanConstructorRejectsOversizedPackets)
{
    const std::array<std::uint8_t, net::Packet::MAX_SIZE + 1> bytes{};

    EXPECT_THROW(net::Packet packet(bytes), net::PacketError);
}

TEST(Packet, SpanConstructorAcceptsMaximumSizedPackets)
{
    const std::array<std::uint8_t, net::Packet::MAX_SIZE> bytes{};

    EXPECT_NO_THROW(net::Packet packet(bytes));
}

TEST(Packet, FullyReadIsTrueOnlyAtEnd)
{
    const std::array<std::uint8_t, 2> bytes{1, 2};
    net::Packet packet(bytes);
    std::uint8_t value = 0;

    EXPECT_FALSE(packet.fullyRead());
    packet.readRaw(&value, sizeof(value));
    EXPECT_FALSE(packet.fullyRead());
    packet.readRaw(&value, sizeof(value));
    EXPECT_TRUE(packet.fullyRead());
}

TEST(Packet, EmptyPacketIsFullyRead)
{
    EXPECT_TRUE(net::Packet{}.fullyRead());
}
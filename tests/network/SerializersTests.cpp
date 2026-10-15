/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** SerializersTests.cpp
*/

#include <gtest/gtest.h>

#include <array>
#include <cstdint>
#include <limits>
#include <string>
#include <vector>

#include "../../network/include/network/Packet.hpp"

namespace test_types {

struct Record {
    std::uint32_t id;
    float value;

    bool operator==(const Record&) const = default;
};

inline void serialize(net::Packet& packet, const Record& record)
{
    packet << record.id << record.value;
}

inline void deserialize(net::Packet& packet, Record& record)
{
    packet >> record.id >> record.value;
}

} // namespace test_types

static_assert(net::Serializable<std::vector<std::uint32_t>>);
static_assert(!net::Serializable<std::vector<bool>>);

TEST(Serializers, EmptyStringRoundTrips)
{
    const std::string expected;
    net::Packet packet;
    packet << expected;

    std::string actual;
    packet >> actual;

    EXPECT_EQ(actual, expected);
}

TEST(Serializers, MaximumLengthStringRoundTrips)
{
    const std::string expected(255, 'x');
    net::Packet packet;
    packet << expected;

    std::string actual;
    packet >> actual;

    EXPECT_EQ(actual, expected);
}

TEST(Serializers, StringLargerThanOneByteLengthThrows)
{
    const std::string value(256, 'x');
    net::Packet packet;

    EXPECT_THROW(packet << value, net::PacketError);
}

TEST(Serializers, TruncatedStringThrows)
{
    constexpr std::uint8_t declaredSize = 50;
    const std::array<std::uint8_t, 3> bytes{1, 2, 3};
    net::Packet packet;
    packet << declaredSize;
    packet.writeRaw(bytes.data(), bytes.size());

    std::string value;
    EXPECT_THROW(packet >> value, net::PacketError);
}

TEST(Serializers, EmptyVectorRoundTrips)
{
    const std::vector<std::uint32_t> expected;
    net::Packet packet;
    packet << expected;

    std::vector<std::uint32_t> actual;
    packet >> actual;

    EXPECT_EQ(actual, expected);
}

TEST(Serializers, VectorOfSerializableStructsRoundTrips)
{
    const std::vector<test_types::Record> expected{{1, 1.5F}, {2, 2.5F}};
    net::Packet packet;
    packet << expected;

    std::vector<test_types::Record> actual;
    packet >> actual;

    EXPECT_EQ(actual, expected);
}

TEST(Serializers, FloatArrayRoundTrips)
{
    const std::array<float, 4> expected{1.0F, -2.5F, 3.25F, 4.0F};
    net::Packet packet;
    packet << expected;

    std::array<float, 4> actual{};
    packet >> actual;

    EXPECT_EQ(actual, expected);
}

TEST(Serializers, OversizedVectorCountThrowsBeforeAllocation)
{
    constexpr std::uint16_t count = std::numeric_limits<std::uint16_t>::max();
    const std::array<std::uint8_t, 10> bytes{};
    net::Packet packet;
    packet << count;
    packet.writeRaw(bytes.data(), bytes.size());

    std::vector<std::uint32_t> values;
    EXPECT_THROW(packet >> values, net::PacketError);
    EXPECT_EQ(values.capacity(), 0U);
}
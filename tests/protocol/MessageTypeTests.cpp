/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** MessageTypeTests
*/

#include <gtest/gtest.h>

#include <array>
#include <cstdint>
#include <sstream>

#include "rtype/protocol/MessageType.hpp"

using rtype::protocol::MessageType;

TEST(MessageTypeTests, TheNineTypesAreKnown)
{
    constexpr std::array known{MessageType::HELLO,
                               MessageType::INPUT, MessageType::DISCONNECT,
                               MessageType::WELCOME,
                               MessageType::REJECT, MessageType::SNAPSHOT,
                               MessageType::EVENT,
                               MessageType::SHUTDOWN, MessageType::KICK,};

    for (const MessageType type: known)
        EXPECT_TRUE(rtype::protocol::isKnown(type)) << type;
}

TEST(MessageTypeTests, OtherBytesAreNotKnown)
{
    constexpr std::array<std::uint8_t, 5> unknown{0x00, 0x04, 0x80, 0x87, 0xFF};

    for (const std::uint8_t byte: unknown)
        EXPECT_FALSE(rtype::protocol::isKnown(static_cast<MessageType>(byte)))
            << static_cast<unsigned>(byte);
}

TEST(MessageTypeTests, IsKnownIsUsableAtCompileTime)
{
    static_assert(rtype::protocol::isKnown(MessageType::KICK));
    static_assert(!rtype::protocol::isKnown(static_cast<MessageType>(0x42)));
}

TEST(MessageTypeTests, AKnownTypeIsWrittenByName)
{
    std::ostringstream stream;

    stream << MessageType::SNAPSHOT;
    EXPECT_EQ(stream.str(), "SNAPSHOT");
}

TEST(MessageTypeTests, AnUnknownTypeIsWrittenInHexWithoutLeakingTheFlag)
{
    std::ostringstream stream;

    stream << MessageType::SNAPSHOT << ' ' << static_cast<MessageType>(0x42)
        << ' ' << 255;
    EXPECT_EQ(stream.str(), "SNAPSHOT 0x42 255");
}

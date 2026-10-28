/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** DirectionTests
*/

#include <gtest/gtest.h>

#include <array>
#include <cstdint>
#include <string>

#include "rtype/protocol/MessageType.hpp"
#include "rtype/protocol/ProtocolError.hpp"

using namespace rtype::protocol;

namespace {

constexpr std::array CLIENT_TO_SERVER{MessageType::HELLO, MessageType::INPUT,
    MessageType::DISCONNECT};

constexpr std::array SERVER_TO_CLIENT{MessageType::WELCOME,
    MessageType::REJECT, MessageType::SNAPSHOT, MessageType::EVENT,
    MessageType::SHUTDOWN, MessageType::KICK};

void expectCheckTypeFails(MessageType type, Direction expected,
    DecodeError code, const std::string &message)
{
    try {
        checkType(type, expected);
        ADD_FAILURE() << type << " was accepted";
    } catch (const ProtocolError &error) {
        EXPECT_EQ(error.code(), code) << type;
        EXPECT_EQ(std::string(error.what()), message);
    }
}

}

TEST(DirectionTests, HelloInputAndDisconnectGoFromClientToServer)
{
    for (const MessageType type: CLIENT_TO_SERVER) {
        EXPECT_TRUE(isClientToServer(type)) << type;
        EXPECT_FALSE(isServerToClient(type)) << type;
    }
}

TEST(DirectionTests, TheSixOtherTypesGoFromServerToClient)
{
    for (const MessageType type: SERVER_TO_CLIENT) {
        EXPECT_TRUE(isServerToClient(type)) << type;
        EXPECT_FALSE(isClientToServer(type)) << type;
    }
}

TEST(DirectionTests, AnUnknownTypeGoesNowhere)
{
    for (const std::uint8_t byte: std::array<std::uint8_t, 3>{0x04, 0x80, 0xFF}) {
        const auto type = static_cast<MessageType>(byte);

        EXPECT_FALSE(isClientToServer(type)) << type;
        EXPECT_FALSE(isServerToClient(type)) << type;
    }
}

TEST(DirectionTests, TheDirectionIsKnownAtCompileTime)
{
    static_assert(isClientToServer(MessageType::HELLO));
    static_assert(isServerToClient(MessageType::KICK));
    static_assert(!isClientToServer(static_cast<MessageType>(0x80)));
    static_assert(!isServerToClient(static_cast<MessageType>(0x80)));
}

TEST(DirectionTests, EveryTypeIsAcceptedInItsOwnDirection)
{
    for (const MessageType type: CLIENT_TO_SERVER)
        EXPECT_NO_THROW(checkType(type, Direction::CLIENT_TO_SERVER)) << type;
    for (const MessageType type: SERVER_TO_CLIENT)
        EXPECT_NO_THROW(checkType(type, Direction::SERVER_TO_CLIENT)) << type;
}

TEST(DirectionTests, ATypeTravellingTheOtherWayIsRejected)
{
    expectCheckTypeFails(MessageType::SHUTDOWN, Direction::CLIENT_TO_SERVER,
        DecodeError::WRONG_DIRECTION, "SHUTDOWN: wrong direction");
    expectCheckTypeFails(MessageType::KICK, Direction::CLIENT_TO_SERVER,
        DecodeError::WRONG_DIRECTION, "KICK: wrong direction");
    expectCheckTypeFails(MessageType::HELLO, Direction::SERVER_TO_CLIENT,
        DecodeError::WRONG_DIRECTION, "HELLO: wrong direction");
}

TEST(DirectionTests, AnUnknownTypeIsRejectedInBothDirections)
{
    const auto type = static_cast<MessageType>(0x42);

    expectCheckTypeFails(type, Direction::CLIENT_TO_SERVER,
        DecodeError::UNKNOWN_TYPE, "0x42: unknown type");
    expectCheckTypeFails(type, Direction::SERVER_TO_CLIENT,
        DecodeError::UNKNOWN_TYPE, "0x42: unknown type");
}

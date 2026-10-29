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

namespace proto = rtype::protocol;

namespace {

constexpr std::array CLIENT_TO_SERVER{proto::MessageType::HELLO,
    proto::MessageType::INPUT, proto::MessageType::DISCONNECT};

constexpr std::array SERVER_TO_CLIENT{proto::MessageType::WELCOME,
    proto::MessageType::REJECT, proto::MessageType::SNAPSHOT,
    proto::MessageType::EVENT, proto::MessageType::SHUTDOWN,
    proto::MessageType::KICK};

void expectCheckTypeFails(proto::MessageType type, proto::Direction expected,
    proto::DecodeError code, const std::string &message)
{
    try {
        proto::checkType(type, expected);
        ADD_FAILURE() << type << " was accepted";
    } catch (const proto::ProtocolError &error) {
        EXPECT_EQ(error.code(), code) << type;
        EXPECT_EQ(std::string(error.what()), message);
    }
}

}

TEST(DirectionTests, HelloInputAndDisconnectGoFromClientToServer)
{
    for (const proto::MessageType type: CLIENT_TO_SERVER) {
        EXPECT_TRUE(proto::isClientToServer(type)) << type;
        EXPECT_FALSE(proto::isServerToClient(type)) << type;
    }
}

TEST(DirectionTests, TheSixOtherTypesGoFromServerToClient)
{
    for (const proto::MessageType type: SERVER_TO_CLIENT) {
        EXPECT_TRUE(proto::isServerToClient(type)) << type;
        EXPECT_FALSE(proto::isClientToServer(type)) << type;
    }
}

TEST(DirectionTests, AnUnknownTypeGoesNowhere)
{
    for (const std::uint8_t byte: std::array<std::uint8_t, 3>{0x04, 0x80, 0xFF}) {
        const auto type = static_cast<proto::MessageType>(byte);

        EXPECT_FALSE(proto::isClientToServer(type)) << type;
        EXPECT_FALSE(proto::isServerToClient(type)) << type;
    }
}

TEST(DirectionTests, TheDirectionIsKnownAtCompileTime)
{
    static_assert(proto::isClientToServer(proto::MessageType::HELLO));
    static_assert(proto::isServerToClient(proto::MessageType::KICK));
    static_assert(!proto::isClientToServer(static_cast<
        proto::MessageType>(0x80)));
    static_assert(!proto::isServerToClient(static_cast<
        proto::MessageType>(0x80)));
}

TEST(DirectionTests, EveryTypeIsAcceptedInItsOwnDirection)
{
    for (const proto::MessageType type: CLIENT_TO_SERVER)
        EXPECT_NO_THROW(proto::checkType(type,
            proto::Direction::CLIENT_TO_SERVER))
            << type;
    for (const proto::MessageType type: SERVER_TO_CLIENT)
        EXPECT_NO_THROW(proto::checkType(type,
            proto::Direction::SERVER_TO_CLIENT))
            << type;
}

TEST(DirectionTests, ATypeTravellingTheOtherWayIsRejected)
{
    expectCheckTypeFails(proto::MessageType::SHUTDOWN,
        proto::Direction::CLIENT_TO_SERVER, proto::DecodeError::WRONG_DIRECTION,
        "SHUTDOWN: wrong direction");
    expectCheckTypeFails(proto::MessageType::KICK,
        proto::Direction::CLIENT_TO_SERVER, proto::DecodeError::WRONG_DIRECTION,
        "KICK: wrong direction");
    expectCheckTypeFails(proto::MessageType::HELLO,
        proto::Direction::SERVER_TO_CLIENT, proto::DecodeError::WRONG_DIRECTION,
        "HELLO: wrong direction");
}

TEST(DirectionTests, AnUnknownTypeIsRejectedInBothDirections)
{
    const auto type = static_cast<proto::MessageType>(0x42);

    expectCheckTypeFails(type, proto::Direction::CLIENT_TO_SERVER,
        proto::DecodeError::UNKNOWN_TYPE, "0x42: unknown type");
    expectCheckTypeFails(type, proto::Direction::SERVER_TO_CLIENT,
        proto::DecodeError::UNKNOWN_TYPE, "0x42: unknown type");
}

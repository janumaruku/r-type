/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** MessageTestHelpers
*/

#ifndef R_TYPE_MESSAGE_TEST_HELPERS_HPP
#define R_TYPE_MESSAGE_TEST_HELPERS_HPP

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

#include "network/Packet.hpp"
#include "rtype/protocol/ProtocolError.hpp"

namespace protocol_test {

template <class Message>
std::vector<std::uint8_t> encode(const Message &message)
{
    net::Packet packet;

    packet << message;

    const auto bytes = packet.bytes();
    return {bytes.begin(), bytes.end()};
}

template <class Message>
void expectRoundTrip(const Message &message)
{
    const std::vector<std::uint8_t> bytes = encode(message);
    net::Packet packet{bytes};
    Message read;

    packet >> read;
    EXPECT_EQ(read, message);
    EXPECT_TRUE(packet.fullyRead());
}

template <class Message>
void expectEveryPrefixTruncated(const Message &message)
{
    const std::vector<std::uint8_t> bytes = encode(message);

    for (std::size_t length = 0; length < bytes.size(); ++length) {
        net::Packet packet{std::span(bytes).first(length)};
        Message read;

        try {
            packet >> read;
            ADD_FAILURE() << "a prefix of " << length << " bytes was accepted";
        } catch (const net::PacketError &error) {
            EXPECT_EQ(rtype::protocol::classify(error),
                rtype::protocol::DecodeError::TRUNCATED)
                << "prefix of " << length << " bytes";
        }
    }
}

/*
** serialize doesn't validate, so an invalid message is built by encoding a
** struct holding the invalid value.
*/
template <class Message>
void expectOutOfRange(const Message &invalid)
{
    const std::vector<std::uint8_t> bytes = encode(invalid);
    net::Packet packet{bytes};
    Message read;

    try {
        packet >> read;
        ADD_FAILURE() << "an invalid message was accepted";
    } catch (const rtype::protocol::ProtocolError &error) {
        EXPECT_EQ(error.code(), rtype::protocol::DecodeError::OUT_OF_RANGE)
            << error.what();
    }
}

}

#endif //R_TYPE_MESSAGE_TEST_HELPERS_HPP

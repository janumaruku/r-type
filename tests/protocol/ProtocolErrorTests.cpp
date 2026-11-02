/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ProtocolErrorTests
*/

#include <gtest/gtest.h>

#include <cstddef>
#include <sstream>
#include <string>
#include <utility>

#include "rtype/protocol/ProtocolError.hpp"

using rtype::protocol::DecodeError;
using rtype::protocol::ProtocolError;

TEST(ProtocolErrorTests, TheDatagramSizeIsChecked)
{
    for (const std::size_t size: {std::size_t{0}, std::size_t{7}, std::size_t{1201}}) {
        try {
            rtype::protocol::checkDatagramSize(size);
            ADD_FAILURE() << "a datagram of " << size << " bytes was accepted";
        } catch (const ProtocolError &error) {
            EXPECT_EQ(error.code(), DecodeError::BAD_SIZE) << size << " bytes";
        }
    }
}

TEST(ProtocolErrorTests, TheSmallestAndLargestDatagramsAreAccepted)
{
    EXPECT_NO_THROW(rtype::protocol::checkDatagramSize(8));
    EXPECT_NO_THROW(rtype::protocol::checkDatagramSize(1200));
}

TEST(ProtocolErrorTests, ClassifyGivesTheCodeOfAProtocolError)
{
    const ProtocolError error(DecodeError::OUT_OF_RANGE,
        "WELCOME: playerId out of range");

    EXPECT_EQ(rtype::protocol::classify(error), DecodeError::OUT_OF_RANGE);
}

TEST(ProtocolErrorTests, ClassifyGivesTruncatedForAPlainPacketError)
{
    const net::PacketError error("not enough bytes");

    EXPECT_EQ(rtype::protocol::classify(error), DecodeError::TRUNCATED);
}

TEST(ProtocolErrorTests, AProtocolErrorIsCaughtAsAPacketError)
{
    try {
        throw ProtocolError(DecodeError::TRAILING_BYTES, "INPUT: trailing bytes");
    } catch (const net::PacketError &error) {
        EXPECT_STREQ(error.what(), "INPUT: trailing bytes");
        EXPECT_EQ(rtype::protocol::classify(error), DecodeError::TRAILING_BYTES);
        return;
    }
    FAIL() << "the ProtocolError wasn't caught as a net::PacketError";
}

TEST(ProtocolErrorTests, EveryDecodeErrorIsWrittenByName)
{
    const std::pair<DecodeError, std::string> names[]{
        {DecodeError::BAD_SIZE, "BAD_SIZE"},
        {DecodeError::BAD_MAGIC, "BAD_MAGIC"},
        {DecodeError::UNKNOWN_TYPE, "UNKNOWN_TYPE"},
        {DecodeError::WRONG_DIRECTION, "WRONG_DIRECTION"},
        {DecodeError::TRUNCATED, "TRUNCATED"},
        {DecodeError::TRAILING_BYTES, "TRAILING_BYTES"},
        {DecodeError::OUT_OF_RANGE, "OUT_OF_RANGE"},
    };

    for (const auto &[error, name]: names) {
        std::ostringstream stream;

        stream << error;
        EXPECT_EQ(stream.str(), name);
    }
}
